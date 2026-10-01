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
#ifndef __LCD_H_
	#define __LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

	
#include "gb_common.h"

#define ST7789V_CMD_RESET         0x01
#define ST7789V_CMD_GET_SCANLINE  0x45
#define ST7789V_CMD_SLPOUT        0x11
#define ST7789V_CMD_COLMOD        0x3A
#define COLOR_MODE_65K            0x50
#define COLOR_MODE_16BIT          0x05
#define ST7789V_CMD_INVOFF        0x20
#define ST7789V_CMD_INVON         0x21
#define ST7789V_CMD_NORON         0x13
#define ST7789V_CMD_DISPON        0x29
#define ST7789V_CMD_RAMWR         0x2c
#define ST7789V_CMD_RDID          0x04
#define ST7789V_CMD_MADCTL        0x36
    #define ST7789V_MADCTRL_MY        0x80  // y flip
    #define ST7789V_MADCTRL_MX        0x40  // x flip
    #define ST7789V_MADCTRL_MV        0x20  // x/y swap
    #define ST7789V_MADCTRL_ML        0x10  // refresh top to bottom / bottom to top
    #define ST7789V_MADCTRL_RGB       0x08  // RGB/BGR
    #define ST7789V_MADCTRL_MH        0x04  // refresh lfet to right / right to left 
  
#define ST7789V_CMD_CASET       0x2A
#define ST7789V_CMD_RASET       0x2B

void gb_ll_lcd_init();
void LCD_FAST_test(const gb_pixel* u16_pframe_buffer);
uint32_t LCD_last_refresh_delay();

void lcd_refresh();
    //! return count of lcd draw since power on
uint32_t gb_ll_lcd_get_draw_count();

void lcd_clear( uint16_t u16_pix_color );
	// shift all pixels by ryght ( fade off )
void lcd_dpo();
    // return !0 if last refresh completed
uint8_t lcd_refresh_completed();
void lcd_putpixel( uint16_t x, uint16_t y, gb_pixel color );
gb_pixel lcd_getpixel( uint16_t x, uint16_t y);

void lcd_update_pwm(uint16_t u16_duty);

    // RGB 888 to rgb 565
    // BUG EVITE (meme piege que fifo_used(), voir gb_ll_audio.c) :
    // "inline" sans "static" dans un en-tete inclus depuis du C pur
    // (gb_ll_*.c) suit la semantique ISO C99 -- aucune definition
    // externe garantie. Cette fonction est petite et se retrouvait
    // sans doute toujours inlinee en pratique (donc silencieuse jusqu'ici),
    // mais reste le meme risque latent qu'une "undefined reference" a la
    // moindre evolution du code -- corrige preventivement avec "static",
    // sans risque ni changement de comportement, y compris cote C++
    // (gb_graphics.h inclut aussi ce fichier).
static inline uint16_t lcd_color_rgb( uint8_t red, uint8_t green, uint8_t blue )
{
    return ( red>>3 ) | ((green>>2)<<5) | ((blue>>3)<<(5+6)) ;
//    return 0x1f; // red
//    return 0x3f<<5; // green
//    return 0x1f<<(5+6); // blue
}

    //! set hard fps to @u8_fps, from 40 to 100
void lcd_set_fps( uint8_t u8_fps );
	// vertical scrool up
void lcd_scrool_vertical( int16_t lines_count );
typedef enum gamebuino_color {
	color_white		= 0xFFFF,
	color_gray		= 0x84D5,
	color_darkgray	= 0x426A,
	color_black		= 0x0000,
	color_purple	= 0x4012,
	color_pink		= 0x8239,
	color_red		= 0x20FB,
	color_orange	= 0x155F,
	color_brown		= 0x4479,
	color_beige		= 0x96BF,
	color_yellow	= 0x073E,
	color_lightgreen= 0x4670,
	color_green		= 0x5440,
	color_darkblue	= 0x8200,
	color_blue		= 0xCC28,
	color_lightblue	= 0xFDCF
} gamebuino_color_e;

#ifdef __cplusplus
}
#endif


#endif // of __LCD_H_