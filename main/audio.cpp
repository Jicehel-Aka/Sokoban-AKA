/*
 * audio.cpp - see audio.h.
 * SPDX-License-Identifier: MIT
 */
#include "audio.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "gamebuino.h"
#include "platform.h"

extern gb_audio_player g_audio;

namespace audio {

static gb_audio_track_wav s_music;
static gb_audio_track_wav s_fx;
static void* s_lock = nullptr;

static const char* const SFX_FILES[SFX_COUNT] = { "menu", "select", "back", "error", "move", "stageend" };
static int16_t* s_pcm[SFX_COUNT];
static uint32_t s_pcm_n[SFX_COUNT];

static const int MAX_TRACKS = 25;
static char s_track[MAX_TRACKS][40];       // base names, without ".wav"
static int  s_tracks = 0;
static int  s_cur = -1;                    // index in s_track, -1 = none
static bool s_title_mode = false;
static bool s_want_music = false;
static float s_music_vol = 0.5f, s_sfx_vol = 0.8f;
static int s_music_set = 6;

struct Lock { Lock() { plat::mutex_lock(s_lock); } ~Lock() { plat::mutex_unlock(s_lock); } };

static void pump() { Lock l; g_audio.pool(); }

// Loads a canonical 44-byte-header PCM WAV (what gb_audio_track_wav also requires).
static bool load_wav(const char* path, int16_t** out, uint32_t* n)
{
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    uint8_t h[44];
    bool ok = fread(h, 1, 44, f) == 44 && memcmp(h, "RIFF", 4) == 0 && memcmp(h + 8, "WAVE", 4) == 0 &&
              memcmp(h + 36, "data", 4) == 0;
    uint32_t bytes = 0;
    if (ok) bytes = (uint32_t)h[40] | ((uint32_t)h[41] << 8) | ((uint32_t)h[42] << 16) | ((uint32_t)h[43] << 24);
    if (ok && (bytes == 0 || bytes > 1024 * 1024)) ok = false;     // effects are short
    int16_t* buf = nullptr;
    if (ok) {
        buf = (int16_t*)malloc(bytes);
        ok = buf && fread(buf, 1, bytes, f) == bytes;
    }
    fclose(f);
    if (!ok) { free(buf); return false; }
    *out = buf;
    *n = bytes / 2;
    return true;
}

static int cmp_names(const void* a, const void* b) { return strcasecmp((const char*)a, (const char*)b); }

static void scan_music()
{
    char dir[600];
    plat::data_path(dir, sizeof dir, "music");
    DIR* d = opendir(dir);
    if (!d) return;
    while (struct dirent* e = readdir(d)) {
        const size_t len = strlen(e->d_name);
        if (len < 5 || len - 4 >= sizeof s_track[0] || strcasecmp(e->d_name + len - 4, ".wav") != 0) continue;
        if (s_tracks >= MAX_TRACKS) break;
        memcpy(s_track[s_tracks], e->d_name, len - 4);
        s_track[s_tracks][len - 4] = 0;
        ++s_tracks;
    }
    closedir(d);
    qsort(s_track, (size_t)s_tracks, sizeof s_track[0], cmp_names);
}

void init()
{
    s_lock = plat::mutex_create();
    g_audio.add_track(&s_music);
    g_audio.add_track(&s_fx);
    g_audio.set_master_volume(200);

    char path[600], rel[64];
    for (int i = 0; i < SFX_COUNT; ++i) {
        snprintf(rel, sizeof rel, "sound/%s.wav", SFX_FILES[i]);
        plat::data_path(path, sizeof path, rel);
        load_wav(path, &s_pcm[i], &s_pcm_n[i]);
    }
    scan_music();
    set_volumes(s_music_set, 8);
    plat::start_audio_pump(pump);
}

void set_volumes(int music, int sfx)
{
    if (music < 0) music = 0; else if (music > 10) music = 10;
    if (sfx < 0) sfx = 0; else if (sfx > 10) sfx = 10;
    s_music_set = music;
    s_music_vol = music / 10.0f;
    s_sfx_vol = sfx / 10.0f;
    if (!s_lock) return;
    Lock l;
    s_music.set_track_volume(s_music_vol);
    s_fx.set_track_volume(s_sfx_vol);
    if (music == 0) s_music.stop_playing();
}

void play(Sfx s)
{
    if (!s_lock || s < 0 || s >= SFX_COUNT || !s_pcm[s] || s_sfx_vol <= 0.0f) return;
    Lock l;
    s_fx.play_raw(s_pcm[s], s_pcm_n[s]);
}

static void start_track(int idx)
{
    if (idx < 0 || idx >= s_tracks || s_music_vol <= 0.0f) { s_cur = idx; return; }
    char rel[96], path[700];
    snprintf(rel, sizeof rel, "music/%s.wav", s_track[idx]);
    plat::data_path(path, sizeof path, rel);
    Lock l;
    s_music.stop_playing();
    s_music.set_track_volume(s_music_vol);
    s_music.play_wav(path);
    s_cur = idx;
}

static int find_title()
{
    for (int i = 0; i < s_tracks; ++i)
        if (strcasecmp(s_track[i], "title") == 0) return i;
    return -1;
}

void music_title()
{
    s_want_music = true;
    if (s_title_mode && s_cur >= 0) return;
    s_title_mode = true;
    int t = find_title();
    if (t < 0 && s_tracks > 0) t = 0;
    start_track(t);
}

void music_game()
{
    s_want_music = true;
    if (!s_title_mode && s_cur >= 0) return;
    s_title_mode = false;
    music_next();
}

static int step(int from, int dir)
{
    // game tracks = every track but "title"
    if (s_tracks == 0) return -1;
    const int t = find_title();
    int i = from;
    for (int n = 0; n < s_tracks + 1; ++n) {
        i = (i + dir + s_tracks) % s_tracks;
        if (i != t || s_tracks == 1) return i;
    }
    return from;
}

void music_next() { s_title_mode = false; start_track(step(s_cur < 0 ? -1 : s_cur, 1)); s_want_music = true; }
void music_prev() { s_title_mode = false; start_track(step(s_cur < 0 ? 0 : s_cur, -1)); s_want_music = true; }

void music_stop()
{
    s_want_music = false;
    if (!s_lock) return;
    Lock l;
    s_music.stop_playing();
    s_cur = -1;
}

void tick()
{
    if (!s_lock || !s_want_music || s_tracks == 0 || s_music_vol <= 0.0f) return;
    bool playing;
    {
        Lock l;
        playing = s_music.is_playing();
    }
    if (playing) return;
    if (s_title_mode) start_track(s_cur >= 0 ? s_cur : find_title());      // the title loops
    else start_track(step(s_cur, 1));
}

const char* music_name() { return (s_cur >= 0 && s_cur < s_tracks) ? s_track[s_cur] : ""; }

}  // namespace audio
