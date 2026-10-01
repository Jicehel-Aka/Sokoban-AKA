/*
 * app.cpp - screens, input and main loop of Sokoban for Gamebuino AKA.
 *
 * A port of "Sokoban (GP2X)" by Willems Davy (joyrider3774), MIT License.
 * The rules and the level format are re-implemented in engine/ (hardware independent);
 * everything here sits on the Gamebuino-AKA library, so that the very same code runs on the
 * console and in the SDL2 desktop build.
 *
 * SPDX-License-Identifier: MIT
 */
#include "app.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "gamebuino.h"
#include "gb_ll_system.h"

#include "assets/tiles.h"
#include "audio.h"
#include "engine/sok_builtin.h"
#include "engine/sok_edit.h"
#include "engine/sok_game.h"
#include "engine/sok_pack.h"
#include "gfx.h"
#include "i18n.h"
#include "platform.h"
#include "save.h"

extern gb_core g_core;
extern gb_graphics g_gfx;

using i18n::tr;

namespace app {
namespace {

// ================================================================================ colours
struct Palette {
    uint16_t white, black, yellow, orange, green, red, gray, dim, navy, navy2, border, shadow;
    uint16_t bg_top[6], bg_bot[6];
};
Palette P;

void init_palette()
{
    using gfx::rgb;
    P.white = rgb(255, 255, 255);  P.black = rgb(0, 0, 0);
    P.yellow = rgb(255, 222, 90);  P.orange = rgb(240, 150, 50);
    P.green = rgb(110, 220, 120);  P.red = rgb(240, 90, 80);
    P.gray = rgb(190, 196, 210);   P.dim = rgb(110, 118, 140);
    P.navy = rgb(18, 26, 48);      P.navy2 = rgb(34, 46, 80);
    P.border = rgb(120, 150, 220); P.shadow = rgb(6, 8, 16);
    const int tops[6][3] = { {40, 60, 110}, {70, 40, 100}, {30, 90, 90}, {100, 60, 40}, {50, 80, 50}, {90, 50, 70} };
    for (int i = 0; i < 6; ++i) {
        P.bg_top[i] = rgb(tops[i][0], tops[i][1], tops[i][2]);
        P.bg_bot[i] = rgb(tops[i][0] / 3, tops[i][1] / 3, tops[i][2] / 3);
    }
}

// ================================================================================ input
const uint16_t KEYS[] = {
    gb_buttons::KEY_LEFT, gb_buttons::KEY_RIGHT, gb_buttons::KEY_UP, gb_buttons::KEY_DOWN,
    gb_buttons::KEY_A, gb_buttons::KEY_B, gb_buttons::KEY_C, gb_buttons::KEY_D,
    gb_buttons::KEY_L1, gb_buttons::KEY_R1, gb_buttons::KEY_MENU, gb_buttons::KEY_RUN };
constexpr int NKEYS = (int)(sizeof KEYS / sizeof KEYS[0]);
enum { K_LEFT, K_RIGHT, K_UP, K_DOWN, K_A, K_B, K_C, K_D, K_L1, K_R1, K_MENU, K_RUN };

constexpr uint32_t REPEAT_DELAY_MS = 380, REPEAT_RATE_MS = 90;

struct Input {
    bool now[NKEYS] = {}, prev[NKEYS] = {}, rep[NKEYS] = {};
    uint32_t next_rep[NKEYS] = {};

    void poll(uint32_t t)
    {
        const uint16_t st = g_core.buttons.state() | g_core.joystick.state();
        for (int i = 0; i < NKEYS; ++i) {
            prev[i] = now[i];
            now[i] = (st & KEYS[i]) != 0;
            rep[i] = false;
            if (now[i] && !prev[i]) next_rep[i] = t + REPEAT_DELAY_MS;
            else if (now[i] && (int32_t)(t - next_rep[i]) >= 0) { rep[i] = true; next_rep[i] = t + REPEAT_RATE_MS; }
        }
    }
    bool held(int k) const { return now[k]; }
    bool pressed(int k) const { return now[k] && !prev[k]; }
    bool released(int k) const { return !now[k] && prev[k]; }
    bool repeat(int k) const { return pressed(k) || rep[k]; }
};
Input in;

// ================================================================================ state
enum Screen { SC_TITLE, SC_PACKS, SC_LEVELS, SC_PLAY, SC_PAUSE, SC_OPTIONS, SC_CREDITS,
              SC_ED_PACKS, SC_ED_LEVELS, SC_ED_EDIT, SC_ED_MENU };

save::Config cfg;
Screen screen = SC_TITLE;
Screen options_from = SC_TITLE;
uint32_t t_now = 0;
char toast[48] = "";
uint32_t toast_until = 0;

void show_toast(const char* s)
{
    snprintf(toast, sizeof toast, "%s", s);
    toast_until = t_now + 2200;
}

struct PackInfo {
    char file[96];
    char name[48];
    bool builtin;
    bool user;           // made with the editor: SOKOBAN/mylevels/<file>
    int count;           // -1 until the pack has been loaded once
};
constexpr int MAX_PACKS = 64;
PackInfo packs[MAX_PACKS];
int npacks = 0;
int pack_sel = 0, pack_top = 0;
int cur_pack = -1;
sok::Pack pack;
bool pack_ok = false;

int lv_sel = 0, lv_top = 0;
int unl = 1;                    // unlocked levels of the current pack

sok::Game game;
int cur_level = 0;
int tile = 24, ox = 0, oy = 0;
bool level_done = false, done_shown = false;
bool test_mode = false;          // playing a level from the editor (nothing is saved)
uint32_t done_at = 0;

// walking animation
bool anim = false;
uint32_t anim_t0 = 0;
sok::Dir anim_dir = sok::DIR_DOWN;
bool anim_push = false;
int anim_from_x = 0, anim_from_y = 0;
uint32_t last_move_ms = 0;
constexpr uint32_t ANIM_MS = 130, FAST_STEP_MS = 95;

int menu_sel = 0;
int opt_sel = 0;
int credits_page = 0;

// ================================================================================ helpers
void sfx(audio::Sfx s) { audio::play(s); }

void apply_audio_settings() { audio::set_volumes(cfg.music, cfg.sfx); }

void blit(int id, int size, int x, int y)
{
    const uint16_t* atlas = size == 24 ? TILES_24 : (size == 16 ? TILES_16 : TILES_12);
    g_gfx.drawImage((int16_t)x, (int16_t)y, atlas, (uint16_t)size, (uint16_t)(T_COUNT * size), 0,
                    (uint16_t)(id * size), (uint16_t)size, (uint16_t)size, TILE_KEY);
}

void blit_scaled(int id, int x, int y, int dst)
{
    g_gfx.drawImageScaled((int16_t)x, (int16_t)y, (uint16_t)dst, (uint16_t)dst, TILES_24, 24, T_COUNT * 24,
                          0, (uint16_t)(id * 24), 24, 24, TILE_KEY);
}

// Copies at most `maxc` UTF-8 characters.
void fit(const char* src, int maxc, char* dst, size_t n)
{
    size_t o = 0;
    int c = 0;
    while (*src && c < maxc) {
        size_t len = 1;
        const uint8_t b = (uint8_t)*src;
        if (b >= 0xF0) len = 4; else if (b >= 0xE0) len = 3; else if (b >= 0xC0) len = 2;
        for (size_t i = 0; i < len && src[i]; ++i) if (o + 1 < n) dst[o++] = src[i];
        for (size_t i = 0; i < len && *src; ++i) ++src;
        ++c;
    }
    dst[o] = 0;
}

int hash_str(const char* s)
{
    unsigned h = 5381;
    while (*s) h = h * 33 + (unsigned char)*s++;
    return (int)(h % 6);
}

void background()
{
    const int k = (screen == SC_TITLE || cur_pack < 0 || test_mode || screen >= SC_ED_PACKS) ? 0 : hash_str(packs[cur_pack].name);
    gfx::gradient(P.bg_top[k], P.bg_bot[k]);
}

void header(const char* title)
{
    gfx::rect(0, 0, gfx::W, 22, P.navy);
    gfx::rect(0, 22, gfx::W, 1, P.border);
    gfx::text_center(7, title, P.yellow);
}

void footer(const char* hint)
{
    gfx::rect(0, gfx::H - 14, gfx::W, 14, P.navy);
    gfx::rect(0, gfx::H - 14, gfx::W, 1, P.border);
    gfx::text_center(gfx::H - 11, hint, P.gray);
}

// Moves a menu cursor with up/down (wrapping). Returns true when it moved.
bool nav(int& sel, int n)
{
    if (in.repeat(K_DOWN)) { sel = (sel + 1) % n; sfx(audio::SFX_MENU); return true; }
    if (in.repeat(K_UP)) { sel = (sel + n - 1) % n; sfx(audio::SFX_MENU); return true; }
    return false;
}

// ================================================================================ packs
int cmp_pack(const void* a, const void* b) { return strcasecmp(((const PackInfo*)a)->name, ((const PackInfo*)b)->name); }

void scan_packs()
{
    npacks = 0;
    PackInfo& b = packs[npacks++];
    memset(&b, 0, sizeof b);
    snprintf(b.name, sizeof b.name, "%s", sok::BUILTIN_PACK_NAME);
    b.builtin = true;
    b.count = -1;

    const int first = npacks;
    for (int pass = 0; pass < 2; ++pass) {                  // levelpacks/ (shipped) then mylevels/ (editor)
        char dir[600];
        plat::data_path(dir, sizeof dir, pass == 0 ? "levelpacks" : "mylevels");
        DIR* d = opendir(dir);
        if (!d) continue;
        while (struct dirent* e = readdir(d)) {
            const size_t len = strlen(e->d_name);
            if (len < 5 || len >= sizeof packs[0].file) continue;
            const char* ext = e->d_name + len - 4;
            if (strcasecmp(ext, ".sok") != 0 && strcasecmp(ext, ".txt") != 0) continue;
            if (npacks >= MAX_PACKS) break;
            PackInfo& p = packs[npacks++];
            memset(&p, 0, sizeof p);
            snprintf(p.file, sizeof p.file, "%s", e->d_name);
            fit(e->d_name, (int)len - 4, p.name, sizeof p.name > 39 ? 39 : sizeof p.name);
            p.user = pass == 1;
            p.count = -1;
        }
        closedir(d);
    }
    qsort(packs + first, (size_t)(npacks - first), sizeof packs[0], cmp_pack);
}

bool open_pack(int idx)
{
    pack.clear();
    pack_ok = false;
    cur_pack = idx;
    if (idx < 0 || idx >= npacks) return false;
    PackInfo& p = packs[idx];
    if (p.builtin) {
        pack_ok = pack.parse_text(sok::BUILTIN_PACK_TEXT, strlen(sok::BUILTIN_PACK_TEXT));
    } else {
        char rel[160], path[800];
        snprintf(rel, sizeof rel, "%s/%s", p.user ? "mylevels" : "levelpacks", p.file);
        plat::data_path(path, sizeof path, rel);
        pack_ok = pack.load_file(path);
    }
    if (pack_ok && pack.count() == 0) pack_ok = false;
    p.count = pack_ok ? pack.count() : 0;
    return pack_ok;
}

void refresh_unlocked()
{
    unl = cfg.unlock_all ? pack.count() : save::unlocked(packs[cur_pack].name, pack.count());
}

// ================================================================================ level / game
void layout_board(const sok::Board& b)
{
    const int availh = gfx::H - 16;
    tile = 12;
    if (b.w * 24 <= gfx::W && b.h * 24 <= availh) tile = 24;
    else if (b.w * 16 <= gfx::W && b.h * 16 <= availh) tile = 16;
    ox = (gfx::W - b.w * tile) / 2;
    oy = 16 + (availh - b.h * tile) / 2;
}

bool begin_level(int idx)
{
    sok::Board b;
    if (!pack.get_board(idx, b)) return false;
    game.start(b);
    layout_board(game.board());
    cur_level = idx;
    test_mode = false;
    level_done = done_shown = false;
    anim = false;
    last_move_ms = 0;
    cfg.last_level = (uint16_t)idx;
    snprintf(cfg.last_pack, sizeof cfg.last_pack, "%s", packs[cur_pack].name);
    return true;
}

void enter_levels()
{
    refresh_unlocked();
    lv_sel = unl - 1;
    if (strcmp(cfg.last_pack, packs[cur_pack].name) == 0 && cfg.last_level < unl) lv_sel = cfg.last_level;
    lv_top = 0;
    screen = SC_LEVELS;
}

void ed_back_from_test();

void finish_level()
{
    done_shown = true;
    sfx(audio::SFX_STAGEEND);
    if (test_mode) return;
    if (cur_level + 2 > unl && !cfg.unlock_all) {
        save::set_unlocked(packs[cur_pack].name, cur_level + 2);
    }
    refresh_unlocked();
}

void try_move(sok::Dir d, bool edge)
{
    if (level_done) return;
    if (!edge && t_now - last_move_ms < (cfg.smooth ? ANIM_MS : FAST_STEP_MS)) return;
    if (anim && !edge) return;
    anim = false;                                  // a new key press cuts the previous step short
    const int fx = game.board().px, fy = game.board().py;
    sok::Step st;
    if (!game.move(d, &st)) {
        game.set_facing(d);
        return;
    }
    last_move_ms = t_now;
    anim_from_x = fx; anim_from_y = fy;
    anim_dir = d; anim_push = st.pushed;
    anim_t0 = t_now;
    anim = cfg.smooth != 0;
    if (st.pushed) sfx(audio::SFX_MOVE);
    if (game.solved()) { level_done = true; done_at = t_now; }
}

void update_play()
{
    if (anim && t_now - anim_t0 >= ANIM_MS) anim = false;
    if (level_done) {
        if (!anim && !done_shown && t_now - done_at > 150) finish_level();
        if (done_shown && test_mode) {
            if (in.pressed(K_A) || in.pressed(K_B)) { sfx(audio::SFX_BACK); ed_back_from_test(); }
            else if (in.pressed(K_C)) { sfx(audio::SFX_SELECT); game.restart(); level_done = done_shown = false; }
        } else if (done_shown) {
            if (in.pressed(K_A)) {
                sfx(audio::SFX_SELECT);
                if (cur_level + 1 < pack.count()) begin_level(cur_level + 1);
                else enter_levels();
            } else if (in.pressed(K_B)) { sfx(audio::SFX_BACK); enter_levels(); }
            else if (in.pressed(K_C)) { sfx(audio::SFX_SELECT); begin_level(cur_level); }
        }
        return;
    }
    if (in.pressed(K_LEFT)) try_move(sok::DIR_LEFT, true);
    else if (in.pressed(K_RIGHT)) try_move(sok::DIR_RIGHT, true);
    else if (in.pressed(K_UP)) try_move(sok::DIR_UP, true);
    else if (in.pressed(K_DOWN)) try_move(sok::DIR_DOWN, true);
    else if (in.held(K_LEFT)) try_move(sok::DIR_LEFT, false);
    else if (in.held(K_RIGHT)) try_move(sok::DIR_RIGHT, false);
    else if (in.held(K_UP)) try_move(sok::DIR_UP, false);
    else if (in.held(K_DOWN)) try_move(sok::DIR_DOWN, false);

    if (in.repeat(K_B)) { anim = false; if (game.undo()) sfx(audio::SFX_BACK); else sfx(audio::SFX_ERROR); }
    if (in.repeat(K_A)) { anim = false; if (game.redo()) sfx(audio::SFX_BACK); else sfx(audio::SFX_ERROR); }
    if (in.pressed(K_C)) { anim = false; game.restart(); level_done = false; sfx(audio::SFX_SELECT); }
    if (in.pressed(K_L1)) { audio::music_prev(); show_toast(audio::music_name()); }
    if (in.pressed(K_R1)) { audio::music_next(); show_toast(audio::music_name()); }
}

int player_base(sok::Dir d)
{
    switch (d) { case sok::DIR_LEFT: return 0; case sok::DIR_RIGHT: return 4; case sok::DIR_UP: return 8; default: return 12; }
}

void draw_board()
{
    const sok::Board& b = game.board();
    float t = 1.0f;                                           // animation progress
    if (anim) { t = (float)(t_now - anim_t0) / (float)ANIM_MS; if (t > 1.0f) t = 1.0f; }
    const int dx = sok::dir_dx(anim_dir), dy = sok::dir_dy(anim_dir);
    // the box pushed by the current step: it ends in front of the player
    const int pbx = b.px + dx, pby = b.py + dy;
    const bool box_anim = anim && anim_push;

    for (int y = 0; y < b.h; ++y)
        for (int x = 0; x < b.w; ++x) {
            const uint8_t c = b.cell[y][x];
            const int sx = ox + x * tile, sy = oy + y * tile;
            if (c & sok::F_WALL) { blit(T_WALL, tile, sx, sy); continue; }
            if (!(c & sok::F_INSIDE)) continue;
            blit((c & sok::F_GOAL) ? T_GOAL : T_FLOOR, tile, sx, sy);
        }
    for (int y = 0; y < b.h; ++y)
        for (int x = 0; x < b.w; ++x) {
            if (!(b.cell[y][x] & sok::F_BOX)) continue;
            int sx = ox + x * tile, sy = oy + y * tile;
            if (box_anim && x == pbx && y == pby) {           // slides from one cell behind
                sx -= (int)((1.0f - t) * (float)(dx * tile));
                sy -= (int)((1.0f - t) * (float)(dy * tile));
            }
            blit((b.cell[y][x] & sok::F_GOAL) ? T_BOX_GOAL : T_BOX, tile, sx, sy);
        }
    int px = ox + b.px * tile, py = oy + b.py * tile;
    int frame = 0;
    sok::Dir face = game.facing();
    if (anim) {
        px = ox + anim_from_x * tile + (int)(t * (float)((b.px - anim_from_x) * tile));
        py = oy + anim_from_y * tile + (int)(t * (float)((b.py - anim_from_y) * tile));
        frame = (int)(t * 4.0f); if (frame > 3) frame = 3;
        face = anim_dir;
    }
    blit(T_PLAYER + player_base(face) + frame, tile, px, py);
}

void draw_hud()
{
    gfx::rect(0, 0, gfx::W, 15, P.navy);
    gfx::rect(0, 15, gfx::W, 1, P.border);
    char name[32];
    char buf[64];
    if (test_mode) {
        gfx::text(4, 4, "TEST", P.orange);
    } else {
        fit(packs[cur_pack].name, 14, name, sizeof name);
        gfx::textf(4, 4, P.yellow, "%s", name);
        snprintf(buf, sizeof buf, "%d/%d", cur_level + 1, pack.count());
        gfx::text(4 + 8 * 15, 4, buf, P.white);
    }
    snprintf(buf, sizeof buf, "%s %d  %s %d", tr(i18n::S_MOVES), game.moves(), tr(i18n::S_PUSHES), game.pushes());
    gfx::text(gfx::W - gfx::text_width(buf) - 4, 4, buf, P.gray);
}

void draw_play()
{
    background();
    draw_board();
    draw_hud();
    if (level_done && done_shown) {
        const bool last = !test_mode && cur_level + 1 >= pack.count();
        gfx::panel(40, 80, 240, last ? 88 : 76, P.navy, P.yellow);
        gfx::text_center(90, tr(test_mode ? i18n::S_ED_TEST_OK : i18n::S_SOLVED), test_mode ? P.green : P.yellow);
        char buf[64];
        snprintf(buf, sizeof buf, "%s %d   %s %d", tr(i18n::S_MOVES), game.moves(), tr(i18n::S_PUSHES), game.pushes());
        gfx::text_center(108, buf, P.white);
        int y = 126;
        if (last) { gfx::text_center(y, tr(i18n::S_PACK_DONE), P.green); y += 16; }
        gfx::text_center(y, (last || test_mode) ? tr(i18n::S_BACK) : tr(i18n::S_SOLVED_NEXT), P.gray);
    }
}

// ================================================================================ title
void ed_enter();
void update_ed_packs(); void draw_ed_packs();
void update_ed_levels(); void draw_ed_levels();
void update_ed_edit(); void draw_ed_edit();
void update_ed_menu(); void draw_ed_menu();

const i18n::Str TITLE_ITEMS[5] = { i18n::S_PLAY, i18n::S_EDITOR, i18n::S_OPTIONS, i18n::S_CREDITS, i18n::S_QUIT };

void update_title()
{
    nav(menu_sel, 5);
    if (in.pressed(K_A)) {
        sfx(audio::SFX_SELECT);
        switch (menu_sel) {
            case 0: screen = SC_PACKS; break;
            case 1: ed_enter(); break;
            case 2: options_from = SC_TITLE; opt_sel = 0; screen = SC_OPTIONS; break;
            case 3: credits_page = 0; screen = SC_CREDITS; break;
            default: save::save_config(cfg); plat::return_to_loader();
        }
    }
}

void draw_title()
{
    background();
    // a little scene: boxes, goals and the player
    gfx::rect(0, 150, gfx::W, 2, P.border);
    gfx::text_scaled_center(24, "SOKOBAN", P.shadow, 5);
    gfx::text_scaled_center(22, "SOKOBAN", P.yellow, 5);
    gfx::text_center(70, "Gamebuino AKA", P.white);
    const int ty = 96;
    blit_scaled(T_WALL, 64, ty, 48);   blit_scaled(T_BOX, 112, ty, 48);   blit_scaled(T_PLAYER + 4, 160, ty, 48);
    blit_scaled(T_BOX_GOAL, 208, ty, 48);

    for (int i = 0; i < 5; ++i) {
        const int y = 156 + i * 12;
        const bool sel = i == menu_sel;
        if (sel) { gfx::rect(100, y - 2, 120, 11, P.navy2); gfx::text(106, y, ">", P.yellow); }
        gfx::text_center(y, tr(TITLE_ITEMS[i]), sel ? P.yellow : P.white);
    }
    gfx::text_center(gfx::H - 20, "Original game (c) Willems Davy - MIT", P.dim);
    gfx::text_center(gfx::H - 10, "AKA port: Jicehel", P.dim);
}

// ================================================================================ packs list
constexpr int PACK_ROWS = 11;

void update_packs()
{
    nav(pack_sel, npacks);
    if (pack_sel < pack_top) pack_top = pack_sel;
    if (pack_sel >= pack_top + PACK_ROWS) pack_top = pack_sel - PACK_ROWS + 1;
    if (in.pressed(K_B)) { sfx(audio::SFX_BACK); screen = SC_TITLE; return; }
    if (in.pressed(K_A)) {
        if (open_pack(pack_sel)) { sfx(audio::SFX_SELECT); enter_levels(); }
        else { sfx(audio::SFX_ERROR); show_toast(tr(i18n::S_LOAD_ERROR)); }
    }
}

void draw_packs()
{
    background();
    header(tr(i18n::S_CHOOSE_PACK));
    if (npacks == 0) gfx::text_center(110, tr(i18n::S_NO_PACKS), P.white);
    for (int r = 0; r < PACK_ROWS; ++r) {
        const int i = pack_top + r;
        if (i >= npacks) break;
        const int y = 30 + r * 17;
        const bool sel = i == pack_sel;
        if (sel) gfx::panel(6, y - 3, gfx::W - 12, 16, P.navy2, P.yellow);
        char nm[40];
        fit(packs[i].name, 26, nm, sizeof nm);
        gfx::text(14, y + 1, nm, sel ? P.yellow : P.white);
        char right[24] = "";
        if (packs[i].count > 0) {
            const int u = cfg.unlock_all ? packs[i].count : save::unlocked(packs[i].name, packs[i].count);
            snprintf(right, sizeof right, "%d/%d", u > packs[i].count ? packs[i].count : u, packs[i].count);
        } else if (packs[i].builtin) {
            snprintf(right, sizeof right, "6");
        }
        gfx::text(gfx::W - 14 - gfx::text_width(right), y + 1, right, sel ? P.white : P.gray);
    }
    if (pack_top > 0) gfx::text(gfx::W - 12, 26, "^", P.yellow);
    if (pack_top + PACK_ROWS < npacks) gfx::text(gfx::W - 12, 30 + PACK_ROWS * 17 - 8, "v", P.yellow);
    footer(tr(i18n::S_HELP_MENU));
}

// ================================================================================ level select
constexpr int LV_COLS = 7, LV_ROWS = 5, LV_CW = 32, LV_CH = 24, LV_X = 8, LV_Y = 30;

void update_levels()
{
    const int maxsel = unl - 1;
    if (in.repeat(K_LEFT) && lv_sel > 0) { --lv_sel; sfx(audio::SFX_MENU); }
    if (in.repeat(K_RIGHT) && lv_sel < maxsel) { ++lv_sel; sfx(audio::SFX_MENU); }
    if (in.repeat(K_UP) && lv_sel - LV_COLS >= 0) { lv_sel -= LV_COLS; sfx(audio::SFX_MENU); }
    if (in.repeat(K_DOWN)) { lv_sel = lv_sel + LV_COLS <= maxsel ? lv_sel + LV_COLS : maxsel; sfx(audio::SFX_MENU); }
    if (in.repeat(K_L1)) { lv_sel = lv_sel >= 5 ? lv_sel - 5 : 0; sfx(audio::SFX_MENU); }
    if (in.repeat(K_R1)) { lv_sel = lv_sel + 5 <= maxsel ? lv_sel + 5 : maxsel; sfx(audio::SFX_MENU); }
    const int row = lv_sel / LV_COLS;
    if (row < lv_top) lv_top = row;
    if (row >= lv_top + LV_ROWS) lv_top = row - LV_ROWS + 1;
    if (in.pressed(K_B)) { sfx(audio::SFX_BACK); screen = SC_PACKS; return; }
    if (in.pressed(K_A)) {
        if (begin_level(lv_sel)) { sfx(audio::SFX_SELECT); screen = SC_PLAY; audio::music_game(); }
        else { sfx(audio::SFX_ERROR); show_toast(tr(i18n::S_LOAD_ERROR)); }
    }
}

void draw_preview(const sok::Board& b, int x, int y, int w, int h)
{
    int s = w / b.w;
    const int s2 = h / b.h;
    if (s2 < s) s = s2;
    if (s < 1) s = 1;
    if (s > 8) s = 8;
    const int x0 = x + (w - b.w * s) / 2, y0 = y + (h - b.h * s) / 2;
    const uint16_t wall = gfx::rgb(120, 126, 150), floor_c = gfx::rgb(52, 60, 84), box = P.orange;
    for (int j = 0; j < b.h; ++j)
        for (int i = 0; i < b.w; ++i) {
            const uint8_t c = b.cell[j][i];
            uint16_t col;
            if (c & sok::F_WALL) col = wall;
            else if (c & sok::F_BOX) col = (c & sok::F_GOAL) ? P.yellow : box;
            else if (i == b.px && j == b.py) col = P.green;
            else if (c & sok::F_GOAL) col = P.red;
            else if (c & sok::F_INSIDE) col = floor_c;
            else continue;
            gfx::rect(x0 + i * s, y0 + j * s, s, s, col);
        }
}

void draw_levels()
{
    background();
    char hd[64];
    fit(packs[cur_pack].name, 26, hd, sizeof hd);
    header(hd);
    for (int r = 0; r < LV_ROWS; ++r)
        for (int c = 0; c < LV_COLS; ++c) {
            const int i = (lv_top + r) * LV_COLS + c;
            if (i >= pack.count()) continue;
            const int x = LV_X + c * LV_CW, y = LV_Y + r * LV_CH;
            const bool sel = i == lv_sel, locked = i >= unl;
            uint16_t fill = locked ? P.navy : (i == unl - 1 && !cfg.unlock_all ? P.navy2 : gfx::rgb(30, 80, 56));
            gfx::panel(x, y, LV_CW - 3, LV_CH - 3, fill, sel ? P.yellow : (locked ? P.dim : P.border));
            char n[16];
            snprintf(n, sizeof n, "%d", i + 1);
            gfx::text(x + (LV_CW - 3 - gfx::text_width(n)) / 2, y + (LV_CH - 3 - 8) / 2, n,
                      locked ? P.dim : (sel ? P.yellow : P.white));
        }
    // preview + details of the selected level
    gfx::panel(238, 30, 76, 117, P.navy, P.border);
    sok::Board b;
    if (pack.get_board(lv_sel, b)) draw_preview(b, 240, 32, 72, 113);
    char title[96], buf[64];
    pack.get_title(lv_sel, title, sizeof title);
    char shown[44];
    if (title[0]) fit(title, 38, shown, sizeof shown);
    else snprintf(shown, sizeof shown, tr(i18n::S_LEVEL_FMT), lv_sel + 1);
    gfx::text(8, 158, shown, P.yellow);
    pack.get_author(lv_sel, title, sizeof title);
    if (title[0]) { fit(title, 38, shown, sizeof shown); gfx::text(8, 172, shown, P.gray); }
    snprintf(buf, sizeof buf, tr(i18n::S_LEVELS_FMT), pack.count());
    gfx::text(8, 186, buf, P.dim);
    if (packs[cur_pack].builtin) gfx::text(8 + 8 * (int)(strlen(buf) + 1), 186, tr(i18n::S_BUILTIN_HINT), P.dim);
    snprintf(buf, sizeof buf, "%d x %d", b.w, b.h);
    gfx::text(gfx::W - 8 - gfx::text_width(buf), 158, buf, P.dim);
    footer(tr(i18n::S_HELP_LEVELS));
}

// ================================================================================ pause / options / credits
const i18n::Str PAUSE_ITEMS[5] = { i18n::S_RESUME, i18n::S_RESTART, i18n::S_LEVEL_SELECT, i18n::S_OPTIONS, i18n::S_TO_TITLE };

void update_pause()
{
    nav(menu_sel, 5);
    if (in.pressed(K_B)) { sfx(audio::SFX_BACK); screen = SC_PLAY; return; }
    if (in.pressed(K_A)) {
        sfx(audio::SFX_SELECT);
        switch (menu_sel) {
            case 0: screen = SC_PLAY; break;
            case 1: game.restart(); level_done = done_shown = false; anim = false; screen = SC_PLAY; break;
            case 2: enter_levels(); break;
            case 3: options_from = SC_PAUSE; opt_sel = 0; screen = SC_OPTIONS; break;
            default: audio::music_title(); menu_sel = 0; screen = SC_TITLE;
        }
    }
}

void draw_pause()
{
    draw_play();
    gfx::panel(70, 60, 180, 24 + 5 * 16 + 4, P.navy, P.yellow);
    gfx::text_center(68, tr(i18n::S_PAUSE), P.yellow);
    for (int i = 0; i < 5; ++i) {
        const int y = 90 + i * 16;
        if (i == menu_sel) { gfx::rect(76, y - 3, 168, 14, P.navy2); gfx::text(80, y, ">", P.yellow); }
        gfx::text_center(y, tr(PAUSE_ITEMS[i]), i == menu_sel ? P.yellow : P.white);
    }
}

constexpr int OPT_N = 6;

void update_options()
{
    nav(opt_sel, OPT_N);
    const int lr = (in.repeat(K_RIGHT) ? 1 : 0) - (in.repeat(K_LEFT) ? 1 : 0);
    const bool ok = in.pressed(K_A);
    switch (opt_sel) {
        case 0:
            if (lr || ok) {
                cfg.lang = (uint8_t)((cfg.lang + (lr ? lr : 1) + i18n::LANG_COUNT) % i18n::LANG_COUNT);
                i18n::set_lang(cfg.lang);
                sfx(audio::SFX_MENU);
            }
            break;
        case 1:
            if (lr) { int v = cfg.music + lr; cfg.music = (uint8_t)(v < 0 ? 0 : (v > 10 ? 10 : v)); apply_audio_settings(); sfx(audio::SFX_MENU); }
            break;
        case 2:
            if (lr) { int v = cfg.sfx + lr; cfg.sfx = (uint8_t)(v < 0 ? 0 : (v > 10 ? 10 : v)); apply_audio_settings(); sfx(audio::SFX_MENU); }
            break;
        case 3: if (lr || ok) { cfg.smooth ^= 1; sfx(audio::SFX_MENU); } break;
        case 4: if (lr || ok) { cfg.unlock_all ^= 1; sfx(audio::SFX_MENU); } break;
        default: if (ok) { save::save_config(cfg); sfx(audio::SFX_BACK); screen = options_from; return; }
    }
    if (in.pressed(K_B)) { save::save_config(cfg); sfx(audio::SFX_BACK); screen = options_from; }
}

void draw_slider(int x, int y, int v)
{
    for (int i = 0; i < 10; ++i) gfx::rect(x + i * 7, y + (i < v ? 0 : 3), 5, i < v ? 8 : 5, i < v ? P.yellow : P.dim);
}

void draw_options()
{
    if (options_from == SC_PAUSE) draw_play(); else background();
    gfx::panel(20, 30, 280, 6 * 22 + 36, P.navy, P.border);
    gfx::text_center(38, tr(i18n::S_OPTIONS), P.yellow);
    const i18n::Str labels[OPT_N] = { i18n::S_OPT_LANGUAGE, i18n::S_OPT_MUSIC, i18n::S_OPT_SFX,
                                      i18n::S_OPT_ANIM, i18n::S_OPT_UNLOCK, i18n::S_BACK };
    for (int i = 0; i < OPT_N; ++i) {
        const int y = 62 + i * 22;
        const bool sel = i == opt_sel;
        if (sel) gfx::rect(26, y - 4, 268, 16, P.navy2);
        gfx::text(32, y, tr(labels[i]), sel ? P.yellow : P.white);
        const int rx = 200;
        switch (i) {
            case 0: gfx::text(rx - 8, y, i18n::lang_name(cfg.lang), sel ? P.yellow : P.gray); break;
            case 1: draw_slider(rx, y, cfg.music); break;
            case 2: draw_slider(rx, y, cfg.sfx); break;
            case 3: gfx::text(rx, y, tr(cfg.smooth ? i18n::S_ON : i18n::S_OFF), sel ? P.yellow : P.gray); break;
            case 4: gfx::text(rx, y, tr(cfg.unlock_all ? i18n::S_ON : i18n::S_OFF), sel ? P.yellow : P.gray); break;
            default: break;
        }
    }
    footer("< >  A  B");
}

struct CreditPage { i18n::Str title; const char* lines[9]; };
const CreditPage CREDITS[] = {
    { i18n::S_CR_TITLE_GAME, { "Sokoban (GP2X)", "(c) 2006-2026 Willems Davy", "joyrider3774", "MIT License", "",
                               "github.com/joyrider3774/Sokoban", "", "Rules and levels format", "re-implemented for AKA" } },
    { i18n::S_CR_TITLE_ART, { "Wall: 1001.com", "  CC BY-SA 3.0", "Floor, player: Kenney", "  CC0", "Box: SpriteAttack", "  CC0",
                              "via opengameart.org", "Sprites resized to 24/16/12", nullptr } },
    { i18n::S_CR_TITLE_SOUND, { "Music (opengameart.org)", "Puzzle Game 3: Eric Matyas", "  CC BY 4.0 - soundimage.org",
                                "Title: migfus20  CC BY 4.0", "Periwinkle: axtoncrolley", "  CC BY-SA 3.0",
                                "Calm BGM: syncopika  CC BY 3.0", nullptr, nullptr } },
    { i18n::S_CR_TITLE_SOUND, { "Sounds", "Stage end: Fupi  CC0", "Select, back: ViRiX Dreamcore", "  CC BY 3.0",
                                "Error: ViRiX Dreamcore", "  CC BY 3.0", "Menu: Tim Mortimer  CC BY 3.0",
                                "Move: Willems Davy", nullptr } },
    { i18n::S_CR_TITLE_LEVELS, { "Aymeric du Peloux", "  Cosmos packs, Picokosmos...", "Evgeniy Grigoriev", "  GRIGoRusha packs",
                                 "Lee J Haywood", "  SokEvo, SokHard, SokWhole, LOMA", "Erim Sever, Dries de Clercq",
                                 "Levels from sokobano.de", nullptr } },
    { i18n::S_CR_TITLE_PORT, { "Gamebuino AKA port: Jicehel", "Gamebuino-AKA library (LGPL)", "  Jean-Marie Papillon",
                               "font8x8: Daniel Hepper (PD)", "", "Same code on PC: SDL2", "Source and licences in", "the project CREDITS.md",
                               nullptr } },
};
constexpr int N_CREDITS = (int)(sizeof CREDITS / sizeof CREDITS[0]);

void update_credits()
{
    if (in.repeat(K_RIGHT) || in.pressed(K_A)) { credits_page = (credits_page + 1) % N_CREDITS; sfx(audio::SFX_MENU); }
    if (in.repeat(K_LEFT)) { credits_page = (credits_page + N_CREDITS - 1) % N_CREDITS; sfx(audio::SFX_MENU); }
    if (in.pressed(K_B)) { sfx(audio::SFX_BACK); screen = SC_TITLE; }
}

void draw_credits()
{
    background();
    const CreditPage& p = CREDITS[credits_page];
    header(tr(p.title));
    for (int i = 0; i < 9 && p.lines[i]; ++i) gfx::text(16, 36 + i * 16, p.lines[i], p.lines[i][0] == ' ' ? P.gray : P.white);
    char pg[16];
    snprintf(pg, sizeof pg, tr(i18n::S_PAGE_FMT), credits_page + 1, N_CREDITS);
    gfx::text(gfx::W - 12 - gfx::text_width(pg), 8, pg, P.dim);
    footer("< >  B");
}

// ================================================================================ level editor
// Packs made here live in SOKOBAN/mylevels/MYPACKn.sok (the shipped packs are never touched) and
// show up in the normal pack list. A level is a 26 x 15 character grid (sok::Grid).
constexpr int ED_MAX_LEVELS = 100;
constexpr int ED_TILE = 12;
constexpr int ED_OX = (gfx::W - sok::MAX_W * ED_TILE) / 2, ED_OY = 20;

sok::Grid ed_lv[ED_MAX_LEVELS];
sok::Grid ed_backup;
int  ed_count = 0;
char ed_file[96] = "";            // file name inside mylevels/
char ed_name[64] = "";
char ed_author[64] = "";
int  ed_sel = 0, ed_top = 0;      // level list
int  ed_idx = -1;                 // level being edited
bool ed_new_level = false;
int  ed_cx = 13, ed_cy = 7;       // cursor
int  ed_part = 1;                 // sok::Part index
int  ed_pack_sel = 0, ed_pack_top = 0;
int  ed_menu_sel = 0;
uint32_t ed_delete_until = 0;
int  ed_packs[MAX_PACKS];         // indexes in packs[] of the editable (user) packs
int  ed_npacks = 0;

void ed_build_pack_list()
{
    ed_npacks = 0;
    for (int i = 0; i < npacks; ++i) if (packs[i].user) ed_packs[ed_npacks++] = i;
}

void ed_enter()
{
    ed_build_pack_list();
    ed_pack_sel = ed_pack_top = 0;
    screen = SC_ED_PACKS;
}

const char* status_text(sok::ParseStatus st)
{
    using sok::ParseStatus;
    switch (st) {
        case ParseStatus::Empty: return tr(i18n::S_ST_EMPTY);
        case ParseStatus::TooBig: return tr(i18n::S_ST_TOOBIG);
        case ParseStatus::NoPlayer: return tr(i18n::S_ST_NOPLAYER);
        case ParseStatus::ManyPlayers: return tr(i18n::S_ST_MANYPLAYERS);
        case ParseStatus::NoBox: return tr(i18n::S_ST_NOBOX);
        case ParseStatus::NoGoal: return tr(i18n::S_ST_NOGOAL);
        case ParseStatus::BoxGoalMismatch: return tr(i18n::S_ST_MISMATCH);
        default: return "";
    }
}

bool ed_write()
{
    char rel[160], path[800];
    plat::data_path(path, sizeof path, "mylevels");
    plat::make_dir(path);
    snprintf(rel, sizeof rel, "mylevels/%s", ed_file);
    plat::data_path(path, sizeof path, rel);
    return sok::write_sok(path, ed_name, ed_author, ed_lv, ed_count);
}

// Loads packs[idx] (a user pack) into the editor. Empty/invalid levels of the file are dropped on the next save.
bool ed_load(int idx)
{
    if (!open_pack(idx)) return false;
    if (pack.count() > ED_MAX_LEVELS) { show_toast(tr(i18n::S_ED_PACK_TOO_BIG)); return false; }
    ed_count = 0;
    for (int i = 0; i < pack.count(); ++i) {
        sok::Board b;
        if (pack.get_board(i, b) && ed_lv[ed_count].from_board(b)) ++ed_count;
    }
    snprintf(ed_file, sizeof ed_file, "%s", packs[idx].file);
    snprintf(ed_name, sizeof ed_name, "%s", pack.set_name()[0] ? pack.set_name() : packs[idx].name);
    snprintf(ed_author, sizeof ed_author, "%s", pack.author());
    return true;
}

void ed_new_pack()
{
    // MYPACK1.SOK, MYPACK2.SOK ... first name not used yet
    char rel[96], path[800];
    int n = 1;
    for (; n < 1000; ++n) {
        snprintf(rel, sizeof rel, "mylevels/MYPACK%d.sok", n);
        plat::data_path(path, sizeof path, rel);
        if (!plat::file_exists(path)) break;
    }
    snprintf(ed_file, sizeof ed_file, "MYPACK%d.sok", n);
    snprintf(ed_name, sizeof ed_name, "MyPack%d", n);
    ed_author[0] = 0;
    ed_count = 0;
}

void update_ed_packs()
{
    const int n = ed_npacks + 1;                      // first entry: new pack
    nav(ed_pack_sel, n);
    if (ed_pack_sel < ed_pack_top) ed_pack_top = ed_pack_sel;
    if (ed_pack_sel >= ed_pack_top + PACK_ROWS) ed_pack_top = ed_pack_sel - PACK_ROWS + 1;
    if (in.pressed(K_B)) { sfx(audio::SFX_BACK); screen = SC_TITLE; return; }
    if (in.pressed(K_A)) {
        if (ed_pack_sel == 0) ed_new_pack();
        else if (!ed_load(ed_packs[ed_pack_sel - 1])) { sfx(audio::SFX_ERROR); return; }
        sfx(audio::SFX_SELECT);
        ed_sel = ed_top = 0;
        screen = SC_ED_LEVELS;
    }
}

void draw_ed_packs()
{
    background();
    header(tr(i18n::S_ED_PACKS_TITLE));
    for (int r = 0; r < PACK_ROWS; ++r) {
        const int i = ed_pack_top + r;
        if (i > ed_npacks) break;
        const int y = 30 + r * 17;
        const bool sel = i == ed_pack_sel;
        if (sel) gfx::panel(6, y - 3, gfx::W - 12, 16, P.navy2, P.yellow);
        if (i == 0) {
            gfx::text(14, y + 1, "+", P.green);
            gfx::text(30, y + 1, tr(i18n::S_ED_NEW_PACK), sel ? P.yellow : P.green);
        } else {
            char nm[40];
            fit(packs[ed_packs[i - 1]].name, 30, nm, sizeof nm);
            gfx::text(14, y + 1, nm, sel ? P.yellow : P.white);
        }
    }
    footer(tr(i18n::S_HELP_MENU));
}

// ---- level list of the pack being edited
void update_ed_levels()
{
    const int cells = ed_count + (ed_count < ED_MAX_LEVELS ? 1 : 0);
    const int maxsel = cells - 1;
    if (in.repeat(K_LEFT) && ed_sel > 0) { --ed_sel; sfx(audio::SFX_MENU); }
    if (in.repeat(K_RIGHT) && ed_sel < maxsel) { ++ed_sel; sfx(audio::SFX_MENU); }
    if (in.repeat(K_UP) && ed_sel - LV_COLS >= 0) { ed_sel -= LV_COLS; sfx(audio::SFX_MENU); }
    if (in.repeat(K_DOWN)) { ed_sel = ed_sel + LV_COLS <= maxsel ? ed_sel + LV_COLS : maxsel; sfx(audio::SFX_MENU); }
    const int row = ed_sel / LV_COLS;
    if (row < ed_top) ed_top = row;
    if (row >= ed_top + LV_ROWS) ed_top = row - LV_ROWS + 1;
    if (in.pressed(K_B)) {
        sfx(audio::SFX_BACK);
        scan_packs();                                  // a new/changed pack must show in the lists
        ed_enter();
        return;
    }
    if (in.pressed(K_D) && ed_sel < ed_count) {        // delete: press twice
        if (t_now < ed_delete_until) {
            for (int i = ed_sel; i + 1 < ed_count; ++i) ed_lv[i] = ed_lv[i + 1];
            --ed_count;
            ed_delete_until = 0;
            if (ed_count == 0) {                       // nothing left: remove the file
                char rel[160], path[800];
                snprintf(rel, sizeof rel, "mylevels/%s", ed_file);
                plat::data_path(path, sizeof path, rel);
                remove(path);
            } else if (!ed_write()) {
                show_toast(tr(i18n::S_ED_SAVE_FAILED));
            }
            if (ed_sel >= ed_count && ed_sel > 0) --ed_sel;
            sfx(audio::SFX_BACK);
        } else {
            ed_delete_until = t_now + 2500;
            show_toast(tr(i18n::S_ED_DELETE_CONFIRM));
            sfx(audio::SFX_ERROR);
        }
    }
    if (in.pressed(K_A)) {
        ed_idx = ed_sel;
        ed_new_level = ed_sel >= ed_count;
        if (ed_new_level) { ed_idx = ed_count; ed_lv[ed_idx].clear(); ++ed_count; }
        ed_backup = ed_lv[ed_idx];
        ed_cx = 13; ed_cy = 7; ed_part = 1;
        sfx(audio::SFX_SELECT);
        screen = SC_ED_EDIT;
    }
}

void draw_ed_levels()
{
    background();
    char hd[64];
    fit(ed_name, 26, hd, sizeof hd);
    header(hd);
    const int cells = ed_count + (ed_count < ED_MAX_LEVELS ? 1 : 0);
    for (int r = 0; r < LV_ROWS; ++r)
        for (int c = 0; c < LV_COLS; ++c) {
            const int i = (ed_top + r) * LV_COLS + c;
            if (i >= cells) continue;
            const int x = LV_X + c * LV_CW, y = LV_Y + r * LV_CH;
            const bool sel = i == ed_sel, plus = i >= ed_count;
            gfx::panel(x, y, LV_CW - 3, LV_CH - 3, plus ? P.navy : gfx::rgb(30, 80, 56), sel ? P.yellow : P.border);
            char n[16];
            snprintf(n, sizeof n, plus ? "+" : "%d", i + 1);
            gfx::text(x + (LV_CW - 3 - gfx::text_width(n)) / 2, y + (LV_CH - 3 - 8) / 2, n,
                      plus ? P.green : (sel ? P.yellow : P.white));
        }
    gfx::panel(238, 30, 76, 117, P.navy, P.border);
    sok::Board b;
    if (ed_sel < ed_count && ed_lv[ed_sel].validate(&b) == sok::ParseStatus::Ok) draw_preview(b, 240, 32, 72, 113);
    char buf[64];
    snprintf(buf, sizeof buf, tr(i18n::S_LEVELS_FMT), ed_count);
    gfx::text(8, 158, buf, P.gray);
    gfx::text(8, 172, ed_sel < ed_count ? "" : tr(i18n::S_ED_NEW_LEVEL), P.green);
    gfx::text(8, 186, tr(i18n::S_ED_LIST_HELP), P.dim);
    footer(tr(i18n::S_HELP_MENU));
}

// ---- the drawing board
void ed_apply(bool erase)
{
    ed_lv[ed_idx].place(ed_cx, ed_cy, erase ? sok::Part::Floor : (sok::Part)ed_part);
}

void ed_start_test()
{
    sok::Board b;
    const sok::ParseStatus st = ed_lv[ed_idx].validate(&b);
    if (st != sok::ParseStatus::Ok) { sfx(audio::SFX_ERROR); show_toast(status_text(st)); return; }
    game.start(b);
    layout_board(game.board());
    test_mode = true;
    level_done = done_shown = false;
    anim = false;
    last_move_ms = 0;
    sfx(audio::SFX_SELECT);
    screen = SC_PLAY;
}

void ed_back_from_test()
{
    test_mode = false;
    level_done = done_shown = false;
    screen = SC_ED_EDIT;
}

void update_ed_edit()
{
    bool moved = false;
    if (in.repeat(K_LEFT) && ed_cx > 0) { --ed_cx; moved = true; }
    if (in.repeat(K_RIGHT) && ed_cx < sok::MAX_W - 1) { ++ed_cx; moved = true; }
    if (in.repeat(K_UP) && ed_cy > 0) { --ed_cy; moved = true; }
    if (in.repeat(K_DOWN) && ed_cy < sok::MAX_H - 1) { ++ed_cy; moved = true; }
    if (in.pressed(K_L1)) { ed_part = (ed_part + 4) % 5; sfx(audio::SFX_MENU); }       // parts 1..4 + floor(erase)=0
    if (in.pressed(K_R1)) { ed_part = (ed_part + 1) % 5; sfx(audio::SFX_MENU); }
    if (in.pressed(K_A) || (moved && in.held(K_A))) ed_apply(false);                  // hold A and move to paint
    if (in.pressed(K_C) || (moved && in.held(K_C))) ed_apply(true);
    if (in.pressed(K_D)) ed_start_test();
    if (in.pressed(K_B)) { ed_menu_sel = 0; sfx(audio::SFX_MENU); screen = SC_ED_MENU; }
}

uint16_t ed_cell_bg() { return gfx::rgb(24, 30, 48); }

void draw_ed_edit()
{
    background();
    gfx::rect(0, 0, gfx::W, 16, P.navy);
    gfx::rect(0, 15, gfx::W, 1, P.border);
    char buf[64];
    snprintf(buf, sizeof buf, tr(i18n::S_LEVEL_FMT), ed_idx + 1);
    char hd[128], nm[32];
    fit(ed_name, 16, nm, sizeof nm);
    snprintf(hd, sizeof hd, "%s - %s", nm, buf);
    gfx::text(4, 4, hd, P.yellow);
    snprintf(buf, sizeof buf, "%d,%d", ed_cx + 1, ed_cy + 1);
    gfx::text(gfx::W - gfx::text_width(buf) - 4, 4, buf, P.gray);

    gfx::rect(ED_OX - 1, ED_OY - 1, sok::MAX_W * ED_TILE + 2, sok::MAX_H * ED_TILE + 2, P.border);
    gfx::rect(ED_OX, ED_OY, sok::MAX_W * ED_TILE, sok::MAX_H * ED_TILE, ed_cell_bg());
    const sok::Grid& g = ed_lv[ed_idx];
    for (int y = 0; y < sok::MAX_H; ++y)
        for (int x = 0; x < sok::MAX_W; ++x) {
            const char ch = g.c[y][x];
            if (ch == ' ') continue;
            const int sx = ED_OX + x * ED_TILE, sy = ED_OY + y * ED_TILE;
            switch (ch) {
                case '#': blit(T_WALL, ED_TILE, sx, sy); break;
                case '.': blit(T_GOAL, ED_TILE, sx, sy); break;
                case '$': blit(T_FLOOR, ED_TILE, sx, sy); blit(T_BOX, ED_TILE, sx, sy); break;
                case '*': blit(T_GOAL, ED_TILE, sx, sy); blit(T_BOX_GOAL, ED_TILE, sx, sy); break;
                case '@': blit(T_FLOOR, ED_TILE, sx, sy); blit(T_PLAYER + 12, ED_TILE, sx, sy); break;
                case '+': blit(T_GOAL, ED_TILE, sx, sy); blit(T_PLAYER + 12, ED_TILE, sx, sy); break;
                default: break;
            }
        }
    // cursor (blinks)
    if ((t_now / 250) % 2 == 0)
        gfx::frame(ED_OX + ed_cx * ED_TILE - 1, ED_OY + ed_cy * ED_TILE - 1, ED_TILE + 2, ED_TILE + 2, P.yellow);
    else
        gfx::frame(ED_OX + ed_cx * ED_TILE, ED_OY + ed_cy * ED_TILE, ED_TILE, ED_TILE, P.white);

    // part palette
    const int py = ED_OY + sok::MAX_H * ED_TILE + 4;
    const i18n::Str names[5] = { i18n::S_ED_ERASE, i18n::S_ED_WALL, i18n::S_ED_GOAL, i18n::S_ED_BOX, i18n::S_ED_PLAYER };
    const int ids[5] = { -1, T_WALL, T_GOAL, T_BOX, T_PLAYER + 12 };
    for (int i = 0; i < 5; ++i) {
        const int x = 8 + i * 20;
        gfx::panel(x - 2, py - 2, 18, 18, i == ed_part ? P.navy2 : P.navy, i == ed_part ? P.yellow : P.border);
        if (ids[i] < 0) { gfx::text(x + 4, py + 4, "x", P.red); }
        else blit_scaled(ids[i], x, py, 14);
    }
    gfx::text(116, py + 4, tr(names[ed_part]), P.yellow);
    gfx::text_center(gfx::H - 10, tr(i18n::S_ED_HELP), P.dim);
}

// ---- menu of the drawing board
const i18n::Str ED_MENU_ITEMS[5] = { i18n::S_RESUME, i18n::S_ED_TEST, i18n::S_ED_SAVE, i18n::S_ED_DISCARD, i18n::S_ED_CLEAR };

void update_ed_menu()
{
    nav(ed_menu_sel, 5);
    if (in.pressed(K_B)) { sfx(audio::SFX_BACK); screen = SC_ED_EDIT; return; }
    if (!in.pressed(K_A)) return;
    switch (ed_menu_sel) {
        case 0: screen = SC_ED_EDIT; break;
        case 1: ed_start_test(); break;
        case 2: {
            const sok::ParseStatus st = ed_lv[ed_idx].validate();
            if (st != sok::ParseStatus::Ok) { sfx(audio::SFX_ERROR); show_toast(status_text(st)); break; }
            if (ed_write()) { sfx(audio::SFX_STAGEEND); show_toast(tr(i18n::S_ED_SAVED)); ed_sel = ed_idx; screen = SC_ED_LEVELS; }
            else { sfx(audio::SFX_ERROR); show_toast(tr(i18n::S_ED_SAVE_FAILED)); }
            break;
        }
        case 3:
            ed_lv[ed_idx] = ed_backup;
            if (ed_new_level) { --ed_count; ed_sel = ed_count > 0 ? ed_count - 1 : 0; } else ed_sel = ed_idx;
            sfx(audio::SFX_BACK);
            screen = SC_ED_LEVELS;
            break;
        default:
            ed_lv[ed_idx].clear();
            sfx(audio::SFX_SELECT);
            screen = SC_ED_EDIT;
    }
}

void draw_ed_menu()
{
    draw_ed_edit();
    gfx::panel(50, 56, 220, 24 + 5 * 16 + 4, P.navy, P.yellow);
    gfx::text_center(64, tr(i18n::S_EDITOR), P.yellow);
    for (int i = 0; i < 5; ++i) {
        const int y = 86 + i * 16;
        if (i == ed_menu_sel) { gfx::rect(56, y - 3, 208, 14, P.navy2); gfx::text(60, y, ">", P.yellow); }
        gfx::text_center(y, tr(ED_MENU_ITEMS[i]), i == ed_menu_sel ? P.yellow : P.white);
    }
}

// ================================================================================ system
uint32_t menu_down_at = 0, combo_at = 0;
bool menu_tainted = false, combo_armed = false;

void system_keys()
{
    // RUN + MENU held for 500 ms returns to the launcher (on release, so the launcher does not see the keys).
    if (in.held(K_RUN) && in.held(K_MENU)) {
        if (!combo_at) combo_at = t_now;
        if (t_now - combo_at >= 500) combo_armed = true;
        menu_tainted = true;
    } else {
        combo_at = 0;
        if (combo_armed && !in.held(K_RUN) && !in.held(K_MENU)) {
            save::save_config(cfg);
            plat::return_to_loader();
        }
        if (!in.held(K_RUN) && !in.held(K_MENU)) combo_armed = false;
    }
    if (in.pressed(K_MENU)) { menu_down_at = t_now; menu_tainted = in.held(K_RUN); }
    if (in.released(K_MENU) && !menu_tainted) {
        const uint32_t held = t_now - menu_down_at;
        if (held >= 1000) {                       // long press: screenshot (taken from the last frame drawn)
            char name[24];
            if (gfx::screenshot(name, sizeof name)) {
                char msg[48];
                snprintf(msg, sizeof msg, "%s %s", tr(i18n::S_SHOT_SAVED), name);
                show_toast(msg);
            } else {
                show_toast(tr(i18n::S_SHOT_FAILED));
            }
        } else if (held < 600) {
            if (screen == SC_PLAY && test_mode) { sfx(audio::SFX_BACK); ed_back_from_test(); }
            else if (screen == SC_PLAY) { menu_sel = 0; screen = SC_PAUSE; sfx(audio::SFX_MENU); }
            else if (screen == SC_PAUSE) { screen = SC_PLAY; sfx(audio::SFX_BACK); }
        }
    }
    if (!in.held(K_MENU)) menu_tainted = false;
}

void draw_toast()
{
    if (t_now >= toast_until || !toast[0]) return;
    const int w = gfx::text_width(toast) + 12;
    gfx::panel((gfx::W - w) / 2, gfx::H - 40, w, 16, P.navy, P.yellow);
    gfx::text_center(gfx::H - 36, toast, P.white);
}

}  // namespace

// ================================================================================ main loop
void run()
{
    g_core.init();
#ifndef AKA_PC
    g_core.joystick.calibrate_center();            // stick assumed at rest
#endif
    init_palette();
    if (save::load_config(cfg)) i18n::set_lang(cfg.lang);
    else i18n::set_lang(cfg.lang = i18n::FR);
    audio::init();
    apply_audio_settings();
    scan_packs();
    audio::music_title();
    printf("[Sokoban] data: %s  packs: %d\n", plat::data_dir(), npacks);

    uint32_t next_frame = g_core.get_millis();
    for (;;) {
        g_core.pool();
        t_now = g_core.get_millis();
        in.poll(t_now);
        system_keys();

        switch (screen) {
            case SC_TITLE:   update_title(); break;
            case SC_PACKS:   update_packs(); break;
            case SC_LEVELS:  update_levels(); break;
            case SC_PLAY:    update_play(); break;
            case SC_PAUSE:   update_pause(); break;
            case SC_OPTIONS: update_options(); break;
            case SC_CREDITS: update_credits(); break;
            case SC_ED_PACKS:  update_ed_packs(); break;
            case SC_ED_LEVELS: update_ed_levels(); break;
            case SC_ED_EDIT:   update_ed_edit(); break;
            case SC_ED_MENU:   update_ed_menu(); break;
        }
        switch (screen) {
            case SC_TITLE:   draw_title(); break;
            case SC_PACKS:   draw_packs(); break;
            case SC_LEVELS:  draw_levels(); break;
            case SC_PLAY:    draw_play(); break;
            case SC_PAUSE:   draw_pause(); break;
            case SC_OPTIONS: draw_options(); break;
            case SC_CREDITS: draw_credits(); break;
            case SC_ED_PACKS:  draw_ed_packs(); break;
            case SC_ED_LEVELS: draw_ed_levels(); break;
            case SC_ED_EDIT:   draw_ed_edit(); break;
            case SC_ED_MENU:   draw_ed_menu(); break;
        }
        draw_toast();
        gfx::present();
        audio::tick();

#ifndef AKA_PC
        next_frame += 33;                          // 30 fps on the console; the PC backend paces the LCD itself
        const uint32_t n = g_core.get_millis();
        if ((int32_t)(next_frame - n) > 0) gb_delay_ms(next_frame - n); else next_frame = n;
#else
        (void)next_frame;
#endif
    }
}

}  // namespace app
