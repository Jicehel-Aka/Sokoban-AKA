/*
 * gfx.cpp - see gfx.h.
 * SPDX-License-Identifier: MIT
 */
#include "gfx.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "gamebuino.h"
#include "assets/font_accents.h"
#include "platform.h"

extern gb_graphics g_gfx;
extern char font8x8_basic[128][8];      // defined by the gamebuino library (public domain font)

namespace gfx {

uint16_t rgb(int r, int g, int b)
{
    if (r < 0) r = 0; else if (r > 255) r = 255;
    if (g < 0) g = 0; else if (g > 255) g = 255;
    if (b < 0) b = 0; else if (b > 255) b = 255;
    return g_gfx.makeColor((uint8_t)r, (uint8_t)g, (uint8_t)b);
}

void clear(uint16_t c) { g_gfx.clear(c); }

void rect(int x, int y, int w, int h, uint16_t c)
{
    if (w <= 0 || h <= 0) return;
    g_gfx.setColor(c);
    g_gfx.fillRect((int16_t)x, (int16_t)y, (int16_t)w, (int16_t)h);
}

void frame(int x, int y, int w, int h, uint16_t c)
{
    rect(x, y, w, 1, c);
    rect(x, y + h - 1, w, 1, c);
    rect(x, y, 1, h, c);
    rect(x + w - 1, y, 1, h, c);
}

void panel(int x, int y, int w, int h, uint16_t fill, uint16_t border)
{
    rect(x, y, w, h, fill);
    frame(x, y, w, h, border);
}

void gradient(uint16_t top, uint16_t bottom)
{
    // The two colours are given in framebuffer order (R | G<<5 | B<<11): split, mix, repack.
    const int r0 = top & 31, g0 = (top >> 5) & 63, b0 = (top >> 11) & 31;
    const int r1 = bottom & 31, g1 = (bottom >> 5) & 63, b1 = (bottom >> 11) & 31;
    const int bands = H / 8;
    for (int i = 0; i < bands; ++i) {
        const int r = r0 + (r1 - r0) * i / (bands - 1);
        const int g = g0 + (g1 - g0) * i / (bands - 1);
        const int b = b0 + (b1 - b0) * i / (bands - 1);
        rect(0, i * 8, W, 8, (uint16_t)(r | (g << 5) | (b << 11)));
    }
}

// --- UTF-8 text ---------------------------------------------------------------------------------

static uint16_t utf8_next(const char*& p)
{
    const uint8_t c = (uint8_t)*p++;
    if (c < 0x80) return c;
    if ((c & 0xE0) == 0xC0 && (*p & 0xC0) == 0x80) {
        const uint16_t cp = (uint16_t)(((c & 0x1F) << 6) | (*p & 0x3F));
        ++p;
        return cp;
    }
    if ((c & 0xF0) == 0xE0) {                       // 3 bytes: not in the font
        if ((p[0] & 0xC0) == 0x80) ++p;
        if ((p[0] & 0xC0) == 0x80) ++p;
        return '?';
    }
    return '?';
}

static const uint8_t* glyph_of(uint16_t cp)
{
    if (cp < 128) return (const uint8_t*)font8x8_basic[cp];
    const uint8_t* g = font_accent_lookup(cp);
    return g ? g : (const uint8_t*)font8x8_basic[(int)'?'];
}

int text_width(const char* s)
{
    int n = 0;
    while (s && *s) { utf8_next(s); ++n; }
    return n * 8;
}

void text_scaled(int x, int y, const char* s, uint16_t c, int scale)
{
    if (!s) return;
    g_gfx.setColor(c);
    while (*s) {
        const uint16_t cp = utf8_next(s);
        if (x >= W) break;
        if (x + 8 * scale > 0) {
            const uint8_t* g = glyph_of(cp);
            for (int gy = 0; gy < 8; ++gy) {
                const uint8_t row = g[gy];
                if (!row) continue;
                for (int gx = 0; gx < 8; ++gx) {
                    if (!(row & (1 << gx))) continue;
                    if (scale == 1) {
                        const int px = x + gx, py = y + gy;
                        if (px >= 0 && px < W && py >= 0 && py < H) g_gfx.drawPixel((int16_t)px, (int16_t)py);
                    } else {
                        g_gfx.fillRect((int16_t)(x + gx * scale), (int16_t)(y + gy * scale), (int16_t)scale, (int16_t)scale);
                    }
                }
            }
        }
        x += 8 * scale;
    }
}

void text(int x, int y, const char* s, uint16_t c) { text_scaled(x, y, s, c, 1); }

void text_shadow(int x, int y, const char* s, uint16_t c, uint16_t shadow)
{
    text(x + 1, y + 1, s, shadow);
    text(x, y, s, c);
}

void text_center(int y, const char* s, uint16_t c) { text((W - text_width(s)) / 2, y, s, c); }

void text_scaled_center(int y, const char* s, uint16_t c, int scale)
{
    text_scaled((W - text_width(s) * scale) / 2, y, s, c, scale);
}

void textf(int x, int y, uint16_t c, const char* fmt, ...)
{
    char buf[96];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    text(x, y, buf, c);
}

void present() { g_gfx.update(); }

// --- screenshot -----------------------------------------------------------------------------------

bool screenshot(char* path_out, int n)
{
    char path[600];
    int idx = -1;
    for (int i = 0; i < 10000; ++i) {
        char name[24];
        snprintf(name, sizeof name, "SHOT%04d.BMP", i);
        plat::save_path(path, sizeof path, name);
        if (!plat::file_exists(path)) { idx = i; break; }
    }
    if (idx < 0) return false;
    plat::make_dir(plat::save_dir());
    FILE* f = fopen(path, "wb");
    if (!f) return false;

    const int row = W * 3, pad = (4 - (row % 4)) % 4, stride = row + pad;
    const uint32_t data = (uint32_t)stride * H, size = 54 + data;
    uint8_t h[54] = {};
    h[0] = 'B'; h[1] = 'M';
    for (int i = 0; i < 4; ++i) h[2 + i] = (uint8_t)(size >> (8 * i));
    h[10] = 54; h[14] = 40;
    h[18] = (uint8_t)W; h[19] = (uint8_t)(W >> 8);
    h[22] = (uint8_t)H; h[23] = (uint8_t)(H >> 8);
    h[26] = 1; h[28] = 24;
    for (int i = 0; i < 4; ++i) h[34 + i] = (uint8_t)(data >> (8 * i));
    bool ok = fwrite(h, 1, 54, f) == 54;

    static uint8_t line[W * 3 + 4];
    for (int y = H - 1; y >= 0 && ok; --y) {
        for (int x = 0; x < W; ++x) {
            const uint16_t p = lcd_getpixel((uint16_t)x, (uint16_t)y);     // R | G<<5 | B<<11
            const int r = p & 31, g = (p >> 5) & 63, b = (p >> 11) & 31;
            line[x * 3 + 0] = (uint8_t)((b << 3) | (b >> 2));
            line[x * 3 + 1] = (uint8_t)((g << 2) | (g >> 4));
            line[x * 3 + 2] = (uint8_t)((r << 3) | (r >> 2));
        }
        memset(line + row, 0, (size_t)pad);
        ok = fwrite(line, 1, (size_t)stride, f) == (size_t)stride;
    }
    fclose(f);
    if (path_out && n > 0) snprintf(path_out, (size_t)n, "SHOT%04d.BMP", idx);
    return ok;
}

}  // namespace gfx
