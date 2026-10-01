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
 - Jean-Marie Papillon
*/
#include "stdint.h"
#include "gb_ll_lcd.h"
#pragma once

class gb_graphics {
    public:
    gb_graphics();
     ~gb_graphics();
        //! clear screen with requested color
    void clear(uint16_t color);
        //! clear screen with current pen color (set by use setColor())
    void clear();
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
	void drawFastVLine(int16_t x, int16_t y, int16_t h);
	void drawFastHLine(int16_t x, int16_t y, int16_t w);
	void drawRect(int16_t x, int16_t y, int16_t w, int16_t h);
	void fillRect(int16_t x, int16_t y, int16_t w, int16_t h);
	void drawCircle(int16_t x0, int16_t y0, int16_t r);
	void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername);
	void fillCircle(int16_t x0, int16_t y0, int16_t r);
	void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta);
	void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
	void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
	void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius);
	void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius);
        // draw a simple pixel at corinates with current pen color (use setColor to change)
    inline void drawPixel( int16_t x, int16_t y, uint16_t u16_color ) {
        lcd_putpixel( x, y, u16_color );
    }
        // draw a simple pixel at corinates with requested color
    inline void drawPixel( int16_t x, int16_t y ) {
        lcd_putpixel( x, y, u16_color_pen );
    }
        // set current draw color for shapes and text
    inline void setColor( uint16_t u16_color ) {
        u16_color_pen = u16_color;
    }
        // make color from tree 8 bits RGB components (0..255)
    inline uint16_t makeColor(uint8_t red, uint8_t green, uint8_t blue) {
        return lcd_color_rgb(red, green, blue);
    }
        // draw ASCII char at location
    void draw_char( uint16_t x, uint16_t y, char c );
        // move text cursor to new pos
    void move_cursor( uint16_t x, uint16_t y );
        // draw a text string on screen
    void print_str(const char* string );
        // draw formated string to screen
    void printf(const char *pc_format, ...);
        //! set brigntness, 0=off, 255=max
    void set_backlight(uint16_t u16_duty);
        //! get brigntness, 0=off, 255=max
    uint16_t get_backlight();
        //! set as progressive percent 0..100
    void set_backlight_percent(uint8_t u8_percent);
        //! get as progressive percent 0..100
    uint8_t get_backlight_percent();
        //! set hard fps to @u8_fps, from 40 to 100
    void set_refresh_rate( uint8_t u8_fps );
	    //! return fps for last sec
    float get_fps();

        //! trasnfert screen buffer to screen
    void update();

        // --- Ajoute par Claude (session Dark & Under), a la demande de
        // Jicehel : "remettre a jour la librairie" pour ne pas regresser
        // par rapport a la version deja utilisee par poa_aka -- famille
        // de blit d'image haut niveau, absente de cette version de
        // gamebuino mais deja ecrite, testee et utilisee par POA sur
        // materiel reel (fichier gb_graphics_image.cpp, meme dossier).
        // Couleurs attendues dans l'ordre natif du framebuffer (celui
        // produit par makeColor()/lcd_color_rgb()). Chaque appel est
        // decoupe (clippe) aux bords de l'ecran -- coordonnees negatives
        // ou hors-ecran toujours sures.

        //! Draw a full image. (x,y) is the top-left corner on screen.
        //! @param pixels row-major width*height 16-bit buffer.
    void drawImage(int16_t x, int16_t y,
                   const uint16_t* pixels, uint16_t width, uint16_t height);

        //! Same as above, using a transparent color key: every source pixel
        //! equal to @p transparent_color is left untouched on screen.
    void drawImage(int16_t x, int16_t y,
                   const uint16_t* pixels, uint16_t width, uint16_t height,
                   uint16_t transparent_color);

        //! Blit a (w x h) region taken from an atlas / spritesheet of size
        //! (atlas_w x atlas_h). Source top-left = (src_x,src_y), destination
        //! top-left = (x,y). Opaque.
    void drawImage(int16_t x, int16_t y,
                   const uint16_t* atlas, uint16_t atlas_w, uint16_t atlas_h,
                   uint16_t src_x, uint16_t src_y, uint16_t w, uint16_t h);

        //! Same as the region blit above, using a transparent color key.
    void drawImage(int16_t x, int16_t y,
                   const uint16_t* atlas, uint16_t atlas_w, uint16_t atlas_h,
                   uint16_t src_x, uint16_t src_y, uint16_t w, uint16_t h,
                   uint16_t transparent_color);

        //! Nearest-neighbor blit of a (src_w x src_h) region of an atlas into
        //! the (dst_w x dst_h) rectangle at (x,y). Lets a low-res asset fill
        //! a bigger screen. Opaque.
    void drawImageScaled(int16_t x, int16_t y, uint16_t dst_w, uint16_t dst_h,
                         const uint16_t* atlas, uint16_t atlas_w, uint16_t atlas_h,
                         uint16_t src_x, uint16_t src_y, uint16_t src_w, uint16_t src_h);

        //! Same as the scaled region blit above, using a transparent color key.
    void drawImageScaled(int16_t x, int16_t y, uint16_t dst_w, uint16_t dst_h,
                         const uint16_t* atlas, uint16_t atlas_w, uint16_t atlas_h,
                         uint16_t src_x, uint16_t src_y, uint16_t src_w, uint16_t src_h,
                         uint16_t transparent_color);

        //! Scaled full image (opaque).
    void drawImageScaled(int16_t x, int16_t y, uint16_t dst_w, uint16_t dst_h,
                         const uint16_t* pixels, uint16_t src_w, uint16_t src_h);

        //! Scaled full image, using a transparent color key.
    void drawImageScaled(int16_t x, int16_t y, uint16_t dst_w, uint16_t dst_h,
                         const uint16_t* pixels, uint16_t src_w, uint16_t src_h,
                         uint16_t transparent_color);

    private:
        uint16_t u16_color_pen;
        uint16_t cursor_x = 0;
        uint16_t cursor_y = 0;
        uint16_t _u16_duty_lcd_pwm;
        uint8_t _u8_percent_lcd_pwm;
        uint32_t u32_last_stat_date = 0;
        uint32_t u32_last_stat_count = 0;
        float f32_fps_stat = 0;
};


