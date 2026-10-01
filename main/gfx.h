/*
 * gfx.h - drawing helpers on top of gb_graphics: UTF-8 text with the accented 8x8 font,
 * panels, a banded gradient background, screenshots.
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdint.h>

namespace gfx {

constexpr int W = 320;
constexpr int H = 240;

uint16_t rgb(int r, int g, int b);

void clear(uint16_t c);
void rect(int x, int y, int w, int h, uint16_t c);
void frame(int x, int y, int w, int h, uint16_t c);
void panel(int x, int y, int w, int h, uint16_t fill, uint16_t border);
void gradient(uint16_t top, uint16_t bottom);          // banded vertical gradient (8 px bands)

int  text_width(const char* utf8);                     // in pixels (one glyph = 8 px)
void text(int x, int y, const char* utf8, uint16_t c);
void text_shadow(int x, int y, const char* utf8, uint16_t c, uint16_t shadow);
void text_center(int y, const char* utf8, uint16_t c);
void text_scaled(int x, int y, const char* utf8, uint16_t c, int scale);   // integer zoom
void text_scaled_center(int y, const char* utf8, uint16_t c, int scale);
void textf(int x, int y, uint16_t c, const char* fmt, ...) __attribute__((format(printf, 4, 5)));

void present();                                        // pushes the framebuffer to the LCD

// Writes the screen as a 24 bit BMP (SHOTxxxx.BMP in the save folder); path receives the file name.
bool screenshot(char* path_out, int n);

}  // namespace gfx
