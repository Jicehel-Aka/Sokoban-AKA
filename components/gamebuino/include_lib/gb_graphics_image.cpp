/*
This file is part of the Gamebuino-AKA library,
Copyright (c) Gamebuino 2026

This is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License (LGPL)
as published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License (LGPL) for more details.

You should have received a copy of the GNU Lesser General Public
License (LGPL) along with the library.
If not, see <http://www.gnu.org/licenses/>.

Authors:
 - gb_graphics::drawImage() high-level image blit family.
   Colors are expected in the native framebuffer order (the order produced
   by gb_graphics::makeColor() / lcd_color_rgb()). Every call is clipped to
   the screen bounds, so negative or out-of-screen coordinates are safe.
*/

#include <string.h>
#include "gb_graphics.h"
#include "gb_common.h"      // framebuffer[], SCREEN_WIDTH, SCREEN_HEIGHT, gb_pixel

// These RGB565 blits target the 16-bit framebuffer configuration.
#ifndef USE_VIDEO_256_INDEXED

// ---- full image ----------------------------------------------------------

void gb_graphics::drawImage(int16_t x, int16_t y,
                            const uint16_t* pixels, uint16_t width, uint16_t height) {
    drawImage(x, y, pixels, width, height, 0, 0, width, height);
}

void gb_graphics::drawImage(int16_t x, int16_t y,
                            const uint16_t* pixels, uint16_t width, uint16_t height,
                            uint16_t transparent_color) {
    drawImage(x, y, pixels, width, height, 0, 0, width, height, transparent_color);
}

// ---- region of an atlas / spritesheet ------------------------------------

void gb_graphics::drawImage(int16_t x, int16_t y,
                            const uint16_t* atlas, uint16_t atlas_w, uint16_t atlas_h,
                            uint16_t src_x, uint16_t src_y, uint16_t w, uint16_t h) {
    (void)atlas_h;
    int dx = x, dy = y, sx = src_x, sy = src_y, iw = w, ih = h;

    // clip to screen
    if (dx < 0) { sx -= dx; iw += dx; dx = 0; }
    if (dy < 0) { sy -= dy; ih += dy; dy = 0; }
    if (dx + iw > SCREEN_WIDTH)  iw = SCREEN_WIDTH  - dx;
    if (dy + ih > SCREEN_HEIGHT) ih = SCREEN_HEIGHT - dy;
    if (iw <= 0 || ih <= 0) return;

    for (int row = 0; row < ih; ++row) {
        const uint16_t* s = atlas + (size_t)(sy + row) * atlas_w + sx;
        gb_pixel*       d = &framebuffer[(size_t)(dy + row) * SCREEN_WIDTH + dx];
        memcpy(d, s, (size_t)iw * sizeof(gb_pixel));   // opaque: fast row copy
    }
}

void gb_graphics::drawImage(int16_t x, int16_t y,
                            const uint16_t* atlas, uint16_t atlas_w, uint16_t atlas_h,
                            uint16_t src_x, uint16_t src_y, uint16_t w, uint16_t h,
                            uint16_t transparent_color) {
    (void)atlas_h;
    int dx = x, dy = y, sx = src_x, sy = src_y, iw = w, ih = h;

    // clip to screen
    if (dx < 0) { sx -= dx; iw += dx; dx = 0; }
    if (dy < 0) { sy -= dy; ih += dy; dy = 0; }
    if (dx + iw > SCREEN_WIDTH)  iw = SCREEN_WIDTH  - dx;
    if (dy + ih > SCREEN_HEIGHT) ih = SCREEN_HEIGHT - dy;
    if (iw <= 0 || ih <= 0) return;

    for (int row = 0; row < ih; ++row) {
        const uint16_t* s = atlas + (size_t)(sy + row) * atlas_w + sx;
        gb_pixel*       d = &framebuffer[(size_t)(dy + row) * SCREEN_WIDTH + dx];
        for (int col = 0; col < iw; ++col) {
            uint16_t c = s[col];
            if (c != transparent_color) d[col] = c;    // color-key transparency
        }
    }
}

// ---- scaled region (nearest-neighbor) ------------------------------------

void gb_graphics::drawImageScaled(int16_t x, int16_t y, uint16_t dst_w, uint16_t dst_h,
                                  const uint16_t* atlas, uint16_t atlas_w, uint16_t atlas_h,
                                  uint16_t src_x, uint16_t src_y, uint16_t src_w, uint16_t src_h) {
    (void)atlas_h;
    if (!dst_w || !dst_h || !src_w || !src_h) return;

    int cx0 = (x < 0) ? 0 : x;
    int cy0 = (y < 0) ? 0 : y;
    int cx1 = (x + dst_w > SCREEN_WIDTH)  ? SCREEN_WIDTH  : (x + dst_w);
    int cy1 = (y + dst_h > SCREEN_HEIGHT) ? SCREEN_HEIGHT : (y + dst_h);

    for (int Y = cy0; Y < cy1; ++Y) {
        int srow = src_y + (int)((long)(Y - y) * src_h / dst_h);
        const uint16_t* s = atlas + (size_t)srow * atlas_w + src_x;
        gb_pixel*       d = &framebuffer[(size_t)Y * SCREEN_WIDTH];
        for (int X = cx0; X < cx1; ++X) {
            int scol = (int)((long)(X - x) * src_w / dst_w);
            d[X] = s[scol];
        }
    }
}

void gb_graphics::drawImageScaled(int16_t x, int16_t y, uint16_t dst_w, uint16_t dst_h,
                                  const uint16_t* atlas, uint16_t atlas_w, uint16_t atlas_h,
                                  uint16_t src_x, uint16_t src_y, uint16_t src_w, uint16_t src_h,
                                  uint16_t transparent_color) {
    (void)atlas_h;
    if (!dst_w || !dst_h || !src_w || !src_h) return;

    int cx0 = (x < 0) ? 0 : x;
    int cy0 = (y < 0) ? 0 : y;
    int cx1 = (x + dst_w > SCREEN_WIDTH)  ? SCREEN_WIDTH  : (x + dst_w);
    int cy1 = (y + dst_h > SCREEN_HEIGHT) ? SCREEN_HEIGHT : (y + dst_h);

    for (int Y = cy0; Y < cy1; ++Y) {
        int srow = src_y + (int)((long)(Y - y) * src_h / dst_h);
        const uint16_t* s = atlas + (size_t)srow * atlas_w + src_x;
        gb_pixel*       d = &framebuffer[(size_t)Y * SCREEN_WIDTH];
        for (int X = cx0; X < cx1; ++X) {
            int scol = (int)((long)(X - x) * src_w / dst_w);
            uint16_t c = s[scol];
            if (c != transparent_color) d[X] = c;
        }
    }
}

// ---- scaled full image (convenience) -------------------------------------

void gb_graphics::drawImageScaled(int16_t x, int16_t y, uint16_t dst_w, uint16_t dst_h,
                                  const uint16_t* pixels, uint16_t src_w, uint16_t src_h) {
    drawImageScaled(x, y, dst_w, dst_h, pixels, src_w, src_h, 0, 0, src_w, src_h);
}

void gb_graphics::drawImageScaled(int16_t x, int16_t y, uint16_t dst_w, uint16_t dst_h,
                                  const uint16_t* pixels, uint16_t src_w, uint16_t src_h,
                                  uint16_t transparent_color) {
    drawImageScaled(x, y, dst_w, dst_h, pixels, src_w, src_h, 0, 0, src_w, src_h,
                    transparent_color);
}

#endif // USE_VIDEO_256_INDEXED
