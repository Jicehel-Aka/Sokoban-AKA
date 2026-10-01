/*
 * gb_ll_pc.cpp - SDL2 replacement of the Gamebuino-AKA low level layer (gb_ll_*).
 *
 * The real mid level library (gb_core, gb_graphics, gb_audio_player, the audio
 * tracks...) is compiled unchanged on the PC; only the functions that talk to the
 * hardware are replaced here:
 *
 *   gb_ll_lcd      -> SDL window showing `framebuffer` (320x240, BGR565 like the ST7789)
 *   gb_ll_audio    -> SDL audio device (44.1 kHz mono, same 512-sample FIFO buffers)
 *   gb_ll_expander -> keyboard / game controller (the 12 console keys)
 *   gb_ll_adc      -> joystick centred (keys do the work), battery 100 %
 *   gb_ll_system   -> millisecond clock and delays
 *   gb_ll_sdcard   -> nothing to mount: the "SD card" is a folder
 *
 * Because the very same gb_graphics.cpp draws into `framebuffer`, a frame looks
 * pixel for pixel like the one the console would show.
 *
 * Scripted mode (tests): AKA_PC_SCRIPT=<file> drives the keys from a text file, uses a
 * virtual 62.5 fps clock and writes screenshots; see tests/pc_smoke.script.
 *
 * SPDX-License-Identifier: MIT (this file). The gamebuino library itself is LGPL.
 */
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "gb_common.h"
#include "gb_ll_adc.h"
#include "gb_ll_audio.h"
#include "gb_ll_expander.h"
#include "gb_ll_i2c.h"
#include "gb_ll_lcd.h"
#include "gb_ll_sdcard.h"
#include "gb_ll_system.h"
#include "pc_backend.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

// ======================================================================== config
namespace {

int         g_scale = 3;
bool        g_fullscreen = false;
std::string g_data_override, g_save_override, g_exe_dir;

// ---- scripted mode -----------------------------------------------------------
struct ScriptEvent { long frame; std::string cmd, arg; };
std::vector<ScriptEvent> g_script;
size_t g_script_pos = 0;
bool   g_scripted = false;
long   g_frame = 0;                 // number of lcd_refresh() calls
uint16_t g_script_keys = 0;
const uint32_t VIRTUAL_FRAME_MS = 16;

void find_exe_dir()
{
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    std::string p(buf, n);
    size_t s = p.find_last_of("\\/");
    g_exe_dir = (s == std::string::npos) ? "." : p.substr(0, s);
#else
    char buf[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        std::string p(buf);
        size_t s = p.find_last_of('/');
        g_exe_dir = (s == std::string::npos) ? "." : p.substr(0, s);
    } else {
        g_exe_dir = ".";
    }
#endif
}

void load_script(const char* path)
{
    FILE* f = fopen(path, "r");
    if (!f) { fprintf(stderr, "AKA_PC_SCRIPT: cannot open %s\n", path); return; }
    char line[512];
    while (fgets(line, sizeof line, f)) {
        char* p = line;
        while (*p == ' ' || *p == '\t') ++p;
        if (*p == '#' || *p == '\n' || *p == '\0' || *p == '\r') continue;
        long frame; char cmd[32] = "", arg[400] = "";
        if (sscanf(p, "%ld %31s %399[^\r\n]", &frame, cmd, arg) >= 2)
            g_script.push_back({frame, cmd, arg});
    }
    fclose(f);
    g_scripted = true;
}

}  // namespace

extern "C" {

void aka_pc_configure(int argc, char** argv)
{
    find_exe_dir();
    if (const char* e = getenv("SOKOBAN_DATA")) g_data_override = e;
    if (const char* e = getenv("SOKOBAN_SAVE")) g_save_override = e;
    if (const char* e = getenv("AKA_PC_SCALE")) g_scale = atoi(e);
    for (int i = 1; i < argc; ++i) {
        const char* a = argv[i];
        if (!strcmp(a, "--fullscreen") || !strcmp(a, "-f")) g_fullscreen = true;
        else if (!strcmp(a, "--scale") && i + 1 < argc) g_scale = atoi(argv[++i]);
        else if (!strcmp(a, "--data") && i + 1 < argc) g_data_override = argv[++i];
        else if (!strcmp(a, "--save") && i + 1 < argc) g_save_override = argv[++i];
        else if (!strcmp(a, "--help") || !strcmp(a, "-h")) {
            printf("Sokoban for Gamebuino AKA - PC version\n"
                   "  --scale N      window size = 320x240 x N (default 3)\n"
                   "  --fullscreen   start full screen (F11 toggles)\n"
                   "  --data DIR     folder holding levelpacks/ sound/ music/ (default: SOKOBAN next to the program)\n"
                   "  --save DIR     folder for the save files (default: the data folder)\n"
                   "Keys: arrows move, Z/Enter/Space=A, X/Backspace=B, C=C, A/PageDown=L1, R/S/PageUp=R1,\n"
                   "      Esc/M/Tab=MENU (hold 1 s = screenshot), Shift=RUN, Shift+Esc 0.5 s = quit,\n"
                   "      F11 full screen, F12 screenshot, Ctrl+Q quit.\n");
            exit(0);
        }
    }
    if (g_scale < 1) g_scale = 1;
    if (g_scale > 8) g_scale = 8;
    if (const char* s = getenv("AKA_PC_SCRIPT")) load_script(s);
}

const char* aka_pc_exe_dir(void) { return g_exe_dir.empty() ? "." : g_exe_dir.c_str(); }
const char* aka_pc_data_dir_override(void) { return g_data_override.empty() ? nullptr : g_data_override.c_str(); }
const char* aka_pc_save_dir_override(void) { return g_save_override.empty() ? nullptr : g_save_override.c_str(); }
int aka_pc_scripted(void) { return g_scripted ? 1 : 0; }

void* aka_pc_mutex_create(void) { return SDL_CreateMutex(); }
void  aka_pc_mutex_lock(void* m) { SDL_LockMutex((SDL_mutex*)m); }
void  aka_pc_mutex_unlock(void* m) { SDL_UnlockMutex((SDL_mutex*)m); }

}  // extern "C"

// ===================================================================== system
extern "C" {

void gb_ll_system_init() {}

uint32_t gb_get_millis()
{
    if (g_scripted) return (uint32_t)g_frame * VIRTUAL_FRAME_MS;
    return SDL_GetTicks();
}

int64_t gb_get_micros()
{
    if (g_scripted) return (int64_t)g_frame * VIRTUAL_FRAME_MS * 1000;
    return (int64_t)(SDL_GetPerformanceCounter() * 1000000ULL / SDL_GetPerformanceFrequency());
}

void gb_delay_ms(uint32_t ms)
{
    if (g_scripted) return;
    SDL_Delay(ms ? ms : 1);
}

void gb_delay_us(int64_t us)
{
    if (g_scripted) return;
    if (us >= 1000) SDL_Delay((Uint32)(us / 1000));
}

}  // extern "C"

// ========================================================================= lcd
namespace {

SDL_Window*   g_win = nullptr;
SDL_Renderer* g_ren = nullptr;
SDL_Texture*  g_tex = nullptr;
uint32_t      g_argb[SCREEN_WIDTH * SCREEN_HEIGHT];
uint32_t      g_draw_count = 0;
uint32_t      g_frame_ms = 16;          // 60 fps
uint32_t      g_last_present = 0;

void write_bmp24(const char* path)
{
    FILE* f = fopen(path, "wb");
    if (!f) { fprintf(stderr, "cannot write %s\n", path); return; }
    const int w = SCREEN_WIDTH, h = SCREEN_HEIGHT, row = w * 3;
    const uint32_t size = 54 + row * h;
    uint8_t hdr[54] = {'B', 'M'};
    auto put32 = [&](int o, uint32_t v) { for (int i = 0; i < 4; ++i) hdr[o + i] = (uint8_t)(v >> (8 * i)); };
    put32(2, size); put32(10, 54); put32(14, 40); put32(18, (uint32_t)w); put32(22, (uint32_t)h);
    hdr[26] = 1; hdr[28] = 24; put32(34, row * h);
    fwrite(hdr, 1, 54, f);
    std::vector<uint8_t> line(row);
    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            const uint16_t v = framebuffer[y * w + x];     // BGR565: red in the low bits
            const uint8_t r = (uint8_t)(((v & 31) << 3) | ((v & 31) >> 2));
            const uint8_t g = (uint8_t)((((v >> 5) & 63) << 2) | (((v >> 5) & 63) >> 4));
            const uint8_t b = (uint8_t)((((v >> 11) & 31) << 3) | (((v >> 11) & 31) >> 2));
            line[x * 3 + 0] = b; line[x * 3 + 1] = g; line[x * 3 + 2] = r;
        }
        fwrite(line.data(), 1, row, f);
    }
    fclose(f);
}

}  // namespace

// The real console defines this; the game draws into it through gb_graphics.
gb_pixel framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

namespace {

void toggle_fullscreen()
{
    if (!g_win) return;
    Uint32 fl = SDL_GetWindowFlags(g_win);
    SDL_SetWindowFullscreen(g_win, (fl & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void take_screenshot()
{
    static int n = 0;
    char name[64];
    snprintf(name, sizeof name, "sokoban_%04d.bmp", ++n);
    write_bmp24(name);
    fprintf(stderr, "screenshot: %s\n", name);
}

// ---- keys ----------------------------------------------------------------------
uint16_t g_keys = 0;                      // current state, EXPANDER_KEY_* bits
SDL_GameController* g_pad = nullptr;
bool g_quit = false;

uint16_t g_pad_keys()
{
    if (!g_pad) return 0;
    uint16_t m = 0;
    auto b = [&](SDL_GameControllerButton k) { return SDL_GameControllerGetButton(g_pad, k) != 0; };
    if (b(SDL_CONTROLLER_BUTTON_DPAD_UP)) m |= EXPANDER_KEY_UP;
    if (b(SDL_CONTROLLER_BUTTON_DPAD_DOWN)) m |= EXPANDER_KEY_DOWN;
    if (b(SDL_CONTROLLER_BUTTON_DPAD_LEFT)) m |= EXPANDER_KEY_LEFT;
    if (b(SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) m |= EXPANDER_KEY_RIGHT;
    const int ax = SDL_GameControllerGetAxis(g_pad, SDL_CONTROLLER_AXIS_LEFTX);
    const int ay = SDL_GameControllerGetAxis(g_pad, SDL_CONTROLLER_AXIS_LEFTY);
    if (ax < -16000) m |= EXPANDER_KEY_LEFT;
    if (ax > 16000) m |= EXPANDER_KEY_RIGHT;
    if (ay < -16000) m |= EXPANDER_KEY_UP;
    if (ay > 16000) m |= EXPANDER_KEY_DOWN;
    if (b(SDL_CONTROLLER_BUTTON_A)) m |= EXPANDER_KEY_A;
    if (b(SDL_CONTROLLER_BUTTON_B)) m |= EXPANDER_KEY_B;
    if (b(SDL_CONTROLLER_BUTTON_X)) m |= EXPANDER_KEY_C;
    if (b(SDL_CONTROLLER_BUTTON_Y)) m |= EXPANDER_KEY_D;
    if (b(SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) m |= EXPANDER_KEY_L1;
    if (b(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) m |= EXPANDER_KEY_R1;
    if (b(SDL_CONTROLLER_BUTTON_START)) m |= EXPANDER_KEY_RUN;
    if (b(SDL_CONTROLLER_BUTTON_BACK)) m |= EXPANDER_KEY_MENU;
    return m;
}

uint16_t keyboard_keys()
{
    const Uint8* s = SDL_GetKeyboardState(nullptr);
    uint16_t m = 0;
    if (s[SDL_SCANCODE_UP]) m |= EXPANDER_KEY_UP;
    if (s[SDL_SCANCODE_DOWN]) m |= EXPANDER_KEY_DOWN;
    if (s[SDL_SCANCODE_LEFT]) m |= EXPANDER_KEY_LEFT;
    if (s[SDL_SCANCODE_RIGHT]) m |= EXPANDER_KEY_RIGHT;
    if (s[SDL_SCANCODE_Z] || s[SDL_SCANCODE_RETURN] || s[SDL_SCANCODE_KP_ENTER] || s[SDL_SCANCODE_SPACE]) m |= EXPANDER_KEY_A;
    if (s[SDL_SCANCODE_X] || s[SDL_SCANCODE_BACKSPACE]) m |= EXPANDER_KEY_B;
    if (s[SDL_SCANCODE_C]) m |= EXPANDER_KEY_C;
    if (s[SDL_SCANCODE_D]) m |= EXPANDER_KEY_D;
    if (s[SDL_SCANCODE_A] || s[SDL_SCANCODE_PAGEDOWN]) m |= EXPANDER_KEY_L1;
    if (s[SDL_SCANCODE_R] || s[SDL_SCANCODE_S] || s[SDL_SCANCODE_PAGEUP]) m |= EXPANDER_KEY_R1;
    if (s[SDL_SCANCODE_ESCAPE] || s[SDL_SCANCODE_M] || s[SDL_SCANCODE_TAB]) m |= EXPANDER_KEY_MENU;
    if (s[SDL_SCANCODE_LSHIFT] || s[SDL_SCANCODE_RSHIFT]) m |= EXPANDER_KEY_RUN;
    return m;
}

uint16_t key_from_name(const std::string& n)
{
    static const struct { const char* name; uint16_t bit; } T[] = {
        {"up", EXPANDER_KEY_UP}, {"down", EXPANDER_KEY_DOWN}, {"left", EXPANDER_KEY_LEFT},
        {"right", EXPANDER_KEY_RIGHT}, {"a", EXPANDER_KEY_A}, {"b", EXPANDER_KEY_B},
        {"c", EXPANDER_KEY_C}, {"d", EXPANDER_KEY_D}, {"l1", EXPANDER_KEY_L1}, {"r1", EXPANDER_KEY_R1},
        {"menu", EXPANDER_KEY_MENU}, {"run", EXPANDER_KEY_RUN}};
    for (auto& t : T) if (n == t.name) return t.bit;
    return 0;
}

void run_script_until_now()
{
    while (g_script_pos < g_script.size() && g_script[g_script_pos].frame <= g_frame) {
        const ScriptEvent& e = g_script[g_script_pos++];
        if (e.cmd == "press") g_script_keys |= key_from_name(e.arg);
        else if (e.cmd == "release") g_script_keys &= (uint16_t)~key_from_name(e.arg);
        else if (e.cmd == "tap") {                       // press now, release 3 frames later
            g_script_keys |= key_from_name(e.arg);
            size_t at = g_script_pos;                    // keep the script sorted by frame
            while (at < g_script.size() && g_script[at].frame <= g_frame + 3) ++at;
            ScriptEvent rel = e;
            rel.cmd = "release"; rel.frame = g_frame + 3;
            g_script.insert(g_script.begin() + (long)at, rel);
        }
        else if (e.cmd == "shot") write_bmp24(e.arg.c_str());
        else if (e.cmd == "quit") g_quit = true;
    }
}

void pump_events()
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_QUIT: g_quit = true; break;
            case SDL_KEYDOWN:
                if (ev.key.repeat) break;
                if (ev.key.keysym.sym == SDLK_F11 ||
                    (ev.key.keysym.sym == SDLK_RETURN && (ev.key.keysym.mod & KMOD_ALT))) toggle_fullscreen();
                else if (ev.key.keysym.sym == SDLK_F12) take_screenshot();
                else if (ev.key.keysym.sym == SDLK_q && (ev.key.keysym.mod & KMOD_CTRL)) g_quit = true;
                break;
            case SDL_CONTROLLERDEVICEADDED:
                if (!g_pad) g_pad = SDL_GameControllerOpen(ev.cdevice.which);
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                if (g_pad && ev.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_pad))) {
                    SDL_GameControllerClose(g_pad); g_pad = nullptr;
                }
                break;
            default: break;
        }
    }
    if (g_quit) { SDL_Quit(); exit(0); }
}

}  // namespace

extern "C" {

void gb_ll_lcd_init()
{
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) != 0) {
            // no audio / controller support: retry with video only
            if (SDL_Init(SDL_INIT_VIDEO) != 0) {
                fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
                exit(1);
            }
        }
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");          // crisp pixels
    g_win = SDL_CreateWindow("Sokoban - Gamebuino AKA", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             SCREEN_WIDTH * g_scale, SCREEN_HEIGHT * g_scale,
                             SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | (g_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    if (!g_win) { fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError()); exit(1); }
    g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_ACCELERATED);
    if (!g_ren) g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_SOFTWARE);
    if (!g_ren) { fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError()); exit(1); }
    SDL_RenderSetLogicalSize(g_ren, SCREEN_WIDTH, SCREEN_HEIGHT);
    g_tex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!g_tex) { fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError()); exit(1); }
    SDL_ShowCursor(SDL_DISABLE);
    memset(framebuffer, 0, sizeof framebuffer);
    for (int i = 0; i < SDL_NumJoysticks(); ++i)
        if (SDL_IsGameController(i)) { g_pad = SDL_GameControllerOpen(i); if (g_pad) break; }
}

uint32_t gb_ll_lcd_get_draw_count() { return g_draw_count; }

void lcd_clear(uint16_t c)
{
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) framebuffer[i] = c;
}

void lcd_putpixel(uint16_t x, uint16_t y, gb_pixel c)
{
    if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT) framebuffer[y * SCREEN_WIDTH + x] = c;
}

gb_pixel lcd_getpixel(uint16_t x, uint16_t y)
{
    return (x < SCREEN_WIDTH && y < SCREEN_HEIGHT) ? framebuffer[y * SCREEN_WIDTH + x] : 0;
}

void lcd_update_pwm(uint16_t) {}
void lcd_set_fps(uint8_t fps)
{
    if (fps >= 20 && fps <= 200) g_frame_ms = 1000u / fps;
}
void lcd_dpo() {}
void LCD_FAST_test(const gb_pixel*) {}
uint32_t LCD_last_refresh_delay() { return 0; }
uint8_t lcd_refresh_completed() { return 1; }

void lcd_scrool_vertical(int16_t lines)
{
    if (lines <= 0 || lines >= SCREEN_HEIGHT) { lcd_clear(0); return; }
    memmove(framebuffer, framebuffer + lines * SCREEN_WIDTH, sizeof(gb_pixel) * SCREEN_WIDTH * (SCREEN_HEIGHT - lines));
    memset(framebuffer + (SCREEN_HEIGHT - lines) * SCREEN_WIDTH, 0, sizeof(gb_pixel) * SCREEN_WIDTH * lines);
}

void lcd_refresh()
{
    ++g_draw_count;
    ++g_frame;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
        const uint16_t v = framebuffer[i];                  // BGR565
        const uint32_t r = (v & 31) << 3, g = ((v >> 5) & 63) << 2, b = ((v >> 11) & 31) << 3;
        g_argb[i] = 0xFF000000u | ((r | (r >> 5)) << 16) | ((g | (g >> 6)) << 8) | (b | (b >> 5));
    }
    SDL_UpdateTexture(g_tex, nullptr, g_argb, SCREEN_WIDTH * 4);
    SDL_RenderClear(g_ren);
    SDL_RenderCopy(g_ren, g_tex, nullptr, nullptr);
    SDL_RenderPresent(g_ren);
    pump_events();

    if (!g_scripted) {                                      // hold the frame rate like the LCD does
        const uint32_t now = SDL_GetTicks();
        if (g_last_present && now - g_last_present < g_frame_ms) SDL_Delay(g_frame_ms - (now - g_last_present));
        g_last_present = SDL_GetTicks();
    }
}

}  // extern "C"

// ===================================================================== input
extern "C" {

int gb_ll_i2c_init() { return GB_OK; }
int gb_ll_expander_init() { return GB_OK; }
void gb_ll_expander_write(uint8_t) {}
void gb_ll_expander_lcd_reset(uint8_t) {}
void gb_ll_expander_lcd_rd(uint8_t) {}
void gb_ll_expander_audio_amplifier_reset(uint8_t) {}
void gb_ll_audio_amp_write(uint8_t, uint8_t) {}
uint8_t gb_ll_audio_amp_read(uint8_t) { return 0; }
uint8_t u8_expander_out_data = 0xFF;

void gb_ll_expander_power_off()
{
    SDL_Quit();
    exit(0);
}

uint16_t gb_ll_expander_read()
{
    if (g_scripted) {
        run_script_until_now();
        if (g_quit) { SDL_Quit(); exit(0); }
        return g_script_keys;
    }
    SDL_PumpEvents();
    g_keys = (uint16_t)(keyboard_keys() | g_pad_keys());
    return g_keys;
}

int gb_ll_adc_init() { return GB_OK; }
int gb_ll_adc_read_vbatt_mv() { return 4000; }
int gb_ll_adc_read_vbatt_percent() { return 100; }
int gb_ll_adc_read_joyx() { return JOYX_MID; }      // keys do the job; stick stays centred
int gb_ll_adc_read_joyy() { return JOYX_MID; }

}  // extern "C"

// ===================================================================== sdcard
extern "C" {
int  gb_ll_sd_init(void) { return GB_OK; }
bool gb_ll_sd_is_mounted(void) { return true; }
}

// ===================================================================== audio
namespace {

const int FIFO = GB_AUDIO_BUFFER_FIFO_COUNT;
const int BUF = GB_AUDIO_BUFFER_SAMPLE_COUNT;

SDL_AudioDeviceID g_adev = 0;
int16_t  g_ring[FIFO][BUF];
volatile int g_used = 0, g_rd = 0, g_wr = 0;     // touched only by the pumping thread
volatile uint8_t g_volume = 255;
void (*g_pump)(void) = nullptr;
uint32_t g_silent_t0 = 0;                          // no device: virtual consumption by time
uint64_t g_silent_consumed = 0;

void silent_drain()
{
    // Without an audio device the FIFO still has to empty at the speed of real playback.
    const uint32_t now = gb_get_millis();
    const uint64_t should = (uint64_t)(now - g_silent_t0) * GB_AUDIO_SAMPLE_RATE / 1000 / BUF;
    while (g_silent_consumed < should && g_used > 0) { g_rd = (g_rd + 1) % FIFO; --g_used; ++g_silent_consumed; }
    if (g_used == 0) g_silent_consumed = should;
}

void SDLCALL audio_callback(void*, Uint8* stream, int len)
{
    int16_t* out = (int16_t*)stream;
    int samples = len / 2;
    if (g_pump) g_pump();                            // lets gb_audio_player::pool() refill the FIFO
    while (samples > 0) {
        if (g_used == 0 && g_pump) g_pump();
        if (g_used == 0) { memset(out, 0, (size_t)samples * 2); break; }
        const int n = samples < BUF ? samples : BUF;
        const int vol = g_volume;
        for (int i = 0; i < n; ++i) out[i] = (int16_t)(g_ring[g_rd][i] * vol / 255);
        g_rd = (g_rd + 1) % FIFO; --g_used;
        out += n; samples -= n;
    }
}

}  // namespace

extern "C" {

void aka_pc_set_audio_pump(void (*fn)(void)) { g_pump = fn; }

int gb_ll_audio_init()
{
    if (g_scripted || !SDL_WasInit(SDL_INIT_AUDIO)) { g_silent_t0 = gb_get_millis(); return GB_OK; }
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = GB_AUDIO_SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = BUF;
    want.callback = audio_callback;
    g_adev = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (!g_adev) {
        fprintf(stderr, "audio disabled: %s\n", SDL_GetError());
        g_silent_t0 = gb_get_millis();
        return GB_OK;
    }
    SDL_PauseAudioDevice(g_adev, 0);
    return GB_OK;
}

void gb_ll_audio_push_buffer(const int16_t* b)
{
    if (g_used >= FIFO) return;
    memcpy(g_ring[g_wr], b, sizeof(int16_t) * BUF);
    g_wr = (g_wr + 1) % FIFO;
    ++g_used;
}

uint32_t gb_ll_audio_fifo_buffer_count() { return FIFO; }
uint32_t gb_ll_audio_fifo_buffer_used() { if (!g_adev) silent_drain(); return (uint32_t)g_used; }
uint32_t gb_ll_audio_fifo_buffer_free() { if (!g_adev) silent_drain(); return (uint32_t)(FIFO - g_used); }
void gb_ll_audio_set_volume(uint8_t v) { g_volume = v; }
void gb_ll_audio_set_vibrator(uint8_t) {}

}  // extern "C"
