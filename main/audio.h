/*
 * audio.h - sound effects and music.
 *
 * Effects are small mono 16 bit 44.1 kHz WAV files (the SOKOBAN/sound folder) loaded into memory at
 * start; music is streamed from the SOKOBAN/music folder. Both are optional: without files the game
 * is simply silent.
 * SPDX-License-Identifier: MIT
 */
#pragma once

namespace audio {

enum Sfx { SFX_MENU, SFX_SELECT, SFX_BACK, SFX_ERROR, SFX_MOVE, SFX_STAGEEND, SFX_COUNT };

void init();
void set_volumes(int music_0_10, int sfx_0_10);

void play(Sfx s);

void music_title();            // plays title.wav
void music_game();             // plays the other tracks in turn
void music_next();
void music_prev();
void music_stop();
void tick();                   // call every frame: starts the next track when one ends
const char* music_name();      // file name of the current track (no extension), "" if none

}  // namespace audio
