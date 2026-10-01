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
#include "gb_graphics.h"
#include "gb_ll_system.h"
#include <stdio.h>
#include <stdarg.h>

#define abs(x) ((x)>0?(x):-(x))
#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif


gb_graphics::gb_graphics()
{
    u16_color_pen = color_white;
}

gb_graphics::~gb_graphics()
{
}

// Bresenham's algorithm - thx wikpedia
void gb_graphics::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		_swap_int16_t(x0, y0);
		_swap_int16_t(x1, y1);
	}

	if (x0 > x1) {
		_swap_int16_t(x0, x1);
		_swap_int16_t(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x0<=x1; x0++) {
		if (steep) {
			drawPixel(y0, x0);
		} else {
			drawPixel(x0, y0);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}


void gb_graphics::drawFastVLine(int16_t x, int16_t y, int16_t h) {
	// Update in subclasses if desired!
	if (h < 0) {
		y += h + 1;
		h = -h;
	}
//	drawLine(x, y, x, y+h-1); don't use bresenham !
// 2026/01/28 : fast implementation

	if ( ( x < 0 ) || (x >=SCREEN_WIDTH) )
		return;
	y = BOUND( y, 0, SCREEN_HEIGHT-1 );
	h = BOUND( h, 0, SCREEN_HEIGHT-y );
	gb_pixel* pdest = &framebuffer[x + y*SCREEN_WIDTH];
	while(h--)
	{
		*pdest = u16_color_pen;
		pdest+=SCREEN_WIDTH;
	}
}


void gb_graphics::drawFastHLine(int16_t x, int16_t y, int16_t w) {
	// Update in subclasses if desired!
	if (w < 0) {
		x += w + 1;
		w = -w;
	}
//	drawLine(x, y, x+w-1, y); don't use bresenham !
// 2026/01/28 : fast implementation
	if ( ( y < 0 ) || (y >=SCREEN_HEIGHT) )
		return;
	x = BOUND( x, 0, SCREEN_WIDTH-1 );
	w = BOUND( w, 0, SCREEN_WIDTH-x );
	gb_pixel* pdest = &framebuffer[x + y*SCREEN_WIDTH];
	while(w--)
		*pdest++ = u16_color_pen;
}

// Draw a rectangle
void gb_graphics::drawRect(int16_t x, int16_t y, int16_t w, int16_t h) {
	drawFastHLine(x, y, w);
	drawFastHLine(x, y+h-1, w);
	drawFastVLine(x, y, h);
	drawFastVLine(x+w-1, y, h);
}

void gb_graphics::clear(uint16_t color) {
	lcd_clear(color);
}

void gb_graphics::clear() {
	lcd_clear(u16_color_pen);
}


void gb_graphics::fillRect(int16_t x, int16_t y, int16_t w, int16_t h) {
	// Update in subclasses if desired!
//	for (int16_t i=x; i<x+w; i++) 
//		drawFastVLine(i, y, h);
		// 2026/01/28 : drawFastHLine more faster
	for (int16_t i=y; i<y+h; i++) 
		drawFastHLine(x, i, w);
}


// Draw a circle outline
void gb_graphics::drawCircle(int16_t x0, int16_t y0, int16_t r) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	drawPixel(x0  , y0+r);
	drawPixel(x0  , y0-r);
	drawPixel(x0+r, y0  );
	drawPixel(x0-r, y0  );

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		drawPixel(x0 + x, y0 + y);
		drawPixel(x0 - x, y0 + y);
		drawPixel(x0 + x, y0 - y);
		drawPixel(x0 - x, y0 - y);
		drawPixel(x0 + y, y0 + x);
		drawPixel(x0 - y, y0 + x);
		drawPixel(x0 + y, y0 - x);
		drawPixel(x0 - y, y0 - x);
	}
}


void gb_graphics::drawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername) {
	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f     += ddF_y;
		}
		x++;
		ddF_x += 2;
		f     += ddF_x;
		if (cornername & 0x4) {
			drawPixel(x0 + x, y0 + y);
			drawPixel(x0 + y, y0 + x);
		}
		if (cornername & 0x2) {
			drawPixel(x0 + x, y0 - y);
			drawPixel(x0 + y, y0 - x);
		}
		if (cornername & 0x8) {
			drawPixel(x0 - y, y0 + x);
			drawPixel(x0 - x, y0 + y);
		}
		if (cornername & 0x1) {
			drawPixel(x0 - y, y0 - x);
			drawPixel(x0 - x, y0 - y);
		}
	}
}

void gb_graphics::fillCircle(int16_t x0, int16_t y0, int16_t r) {
	drawFastVLine(x0, y0-r, 2*r+1);
	fillCircleHelper(x0, y0, r, 3, 0);
}

// Used to do circles and roundrects
void gb_graphics::fillCircleHelper(int16_t x0, int16_t y0, int16_t r,
 uint8_t cornername, int16_t delta) {

	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f     += ddF_y;
		}
		x++;
		ddF_x += 2;
		f     += ddF_x;

		if (cornername & 0x1) {
			drawFastVLine(x0+x, y0-y, 2*y+1+delta);
			drawFastVLine(x0+y, y0-x, 2*x+1+delta);
		}
		if (cornername & 0x2) {
			drawFastVLine(x0-x, y0-y, 2*y+1+delta);
			drawFastVLine(x0-y, y0-x, 2*x+1+delta);
		}
	}
}

// Draw a triangle
void gb_graphics::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
	drawLine(x0, y0, x1, y1);
	drawLine(x1, y1, x2, y2);
	drawLine(x2, y2, x0, y0);
}

// Fill a triangle
void gb_graphics::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {

	int16_t a, b, y, last;

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	if (y0 > y1) {
		_swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
	}
	if (y1 > y2) {
		_swap_int16_t(y2, y1); _swap_int16_t(x2, x1);
	}
	if (y0 > y1) {
		_swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
	}

	if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
		a = b = x0;
		if(x1 < a)      a = x1;
		else if(x1 > b) b = x1;
		if(x2 < a)      a = x2;
		else if(x2 > b) b = x2;
		drawFastHLine(a, y0, b-a+1);
		return;
	}

	int16_t
		dx01 = x1 - x0,
		dy01 = y1 - y0,
		dx02 = x2 - x0,
		dy02 = y2 - y0,
		dx12 = x2 - x1,
		dy12 = y2 - y1;
	int32_t
		sa   = 0,
		sb   = 0;

	// For upper part of triangle, find scanline crossings for segments
	// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	// is included here (and second loop will be skipped, avoiding a /0
	// error there), otherwise scanline y1 is skipped here and handled
	// in the second loop...which also avoids a /0 error here if y0=y1
	// (flat-topped triangle).
	if(y1 == y2) last = y1;   // Include y1 scanline
	else         last = y1-1; // Skip it

	for(y=y0; y<=last; y++) {
		a   = x0 + sa / dy01;
		b   = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		/* longhand:
		a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if(a > b) _swap_int16_t(a,b);
		drawFastHLine(a, y, b-a+1);
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2.  This loop is skipped if y1=y2.
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for(; y<=y2; y++) {
		a   = x1 + sa / dy12;
		b   = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		/* longhand:
		a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if(a > b) _swap_int16_t(a,b);
		drawFastHLine(a, y, b-a+1);
	}
}



// Draw a rounded rectangle
void gb_graphics::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r) {
	// smarter version
	drawFastHLine(x+r  , y    , w-2*r); // Top
	drawFastHLine(x+r  , y+h-1, w-2*r); // Bottom
	drawFastVLine(x    , y+r  , h-2*r); // Left
	drawFastVLine(x+w-1, y+r  , h-2*r); // Right
	// draw four corners
	drawCircleHelper(x+r    , y+r    , r, 1);
	drawCircleHelper(x+w-r-1, y+r    , r, 2);
	drawCircleHelper(x+w-r-1, y+h-r-1, r, 4);
	drawCircleHelper(x+r    , y+h-r-1, r, 8);
}

// Fill a rounded rectangle
void gb_graphics::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r) {
	// smarter version
	fillRect(x+r, y, w-2*r, h);

	// draw four corners
	fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1);
	fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1);
}




void gb_graphics::move_cursor( uint16_t x, uint16_t y )
{
	cursor_x = MIN( x, SCREEN_WIDTH );
	cursor_y = MIN( y, SCREEN_HEIGHT );
}


void gb_graphics::print_str(const char* pc )
{
    while (*pc)
    {
        switch (*pc)
        {
            case '\r':
                cursor_x = 0;
                break;
            case '\n':
                cursor_x = 0;
                cursor_y += 8;
                break;
            default:    
                draw_char( cursor_x, cursor_y, *pc );
                cursor_x += 8;
                break;
        }
        if ( cursor_x >= 320 )
        {
            cursor_x = 0;
            cursor_y += 8;
        }
        if ( cursor_y >= 240 )
        {
			lcd_scrool_vertical( 8 );
            cursor_y -= 8;
        }
        pc++;
    }
}
#include "font8x8_basic.h"
void gb_graphics::draw_char( uint16_t x, uint16_t y, char c )
{
    for ( uint16_t dy = 0 ; dy < 8 ; dy++ )
    {
        uint8_t u8_line = font8x8_basic[(uint8_t)c][dy];
        for ( uint16_t dx = 0 ; dx < 8 ; dx++ )
        {
			if ( u8_line&1 )
            	lcd_putpixel( x+dx, y+dy, u16_color_pen );
            u8_line >>= 1;
        }
    }
}

void gb_graphics::printf(const char *pc_format, ...)
{
va_list ap;
static char pc_message_buffer[256];
	va_start( ap, pc_format );	// init stack ptr from pc_format parameter
	vsnprintf( pc_message_buffer, sizeof(pc_message_buffer), pc_format, ap );	// fromat the string
	va_end( ap );				// release ptr
	print_str(pc_message_buffer);
}


void gb_graphics::set_backlight(uint16_t u16_duty)
{
	_u16_duty_lcd_pwm = u16_duty;
	lcd_update_pwm(u16_duty);
}

static const uint16_t lin_to_grad[101] = { 0, 0,	0,	0,	1,	1,	2,	3,	4,	5,	6,	8,	10,	12,	14,	16,	18,	21,	24,	27,	30,	33,	37,	40,	44,	49,	53,	57,	62,	67,	72,	78,	83,	89,	95,	102,	108,	115,	122,	129,	136,	144,	152,	160,	168,	177,	186,	194,	204,	213,	223,	233,	243,	253,	264,	275,	286,	297,	309,	321,	333,	345,	358,	371,	384,	397,	410,	424,	438,	453,	467,	482,	497,	512,	528,	544,	560,	576,	593,	610,	627,	644,	662,	680,	698,	716,	735,	754,	773,	792,	812,	832,	852,	873,	894,	915,	936,	958,	979,	1002,	1024 };

void gb_graphics::set_backlight_percent(uint8_t u8_percent)
{
	if ( u8_percent == 255  ) // assume as set_backlight_percent(-1);
		u8_percent = 0;
	if ( u8_percent > 100  )
	u8_percent = 100;
	_u8_percent_lcd_pwm = u8_percent;
	set_backlight( lin_to_grad[u8_percent] );
}

uint8_t gb_graphics::get_backlight_percent()
{
	return _u8_percent_lcd_pwm;
}


    //! set hard fps to @u8_fps, from 40 to 100
void gb_graphics::set_refresh_rate( uint8_t u8_fps )
{
	lcd_set_fps(u8_fps);
}


uint16_t gb_graphics::get_backlight()
{
	return _u16_duty_lcd_pwm;
}
	//! return fps for last sec
float gb_graphics::get_fps()
{
	return f32_fps_stat;
}
void gb_graphics::update()
{
	lcd_refresh();

	uint32_t u32_now = gb_get_millis();
	if ( ( u32_now - u32_last_stat_date ) > 1000 )
	{
		uint32_t u32_draw_now = gb_ll_lcd_get_draw_count();
		f32_fps_stat = 1000.0f*(u32_draw_now - u32_last_stat_count)/((float)(u32_now-u32_last_stat_date)) ;
		u32_last_stat_date = u32_now;
		u32_last_stat_count = u32_draw_now;
	}
	while( !lcd_refresh_completed() )
		gb_delay_ms(1);
}

/*
void graphics_basic::drawChar(int16_t x, int16_t y, unsigned char c, uint8_t size) {
	if(!gfxFont) { // 'Classic' built-in font

		if((x >= _width)            || // Clip right
			 (y >= _height)           || // Clip bottom
			 ((x + fontWidth * size - 1) < 0) || // Clip left
			 ((y + fontHeight * size - 1) < 0))   // Clip top
			return;

		if(!_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

		if (c >= 0x80) {
			c -= 0x20;
		}

		for (int8_t i = 0; i < fontWidth; i++) {
			uint8_t line;
			if (i == (fontWidth - 1))
				line = 0x0;
			else
				line = pgm_read_byte(font + (c * (fontWidth - 1)) + i);
			for (int8_t j = 0; j < fontHeight; j++) {
				if (line & 0x1) {
					if (size == 1) // default size
						drawPixel(x + i, y + j);
					else { // big size
						fillRect(x + (i * size), y + (j * size), size, size);
					}
				}
				else if (bgcolor.c != color.c) {
					Color tempcolor = color.c;
					color.c = bgcolor.c;
					if (size == 1) // default size
						drawPixel(x + i, y + j);
					else { // big size
						fillRect(x + i*size, y + j*size, size, size);
					}
					color.c = tempcolor; //restore color to its initial value
				}
				line >>= 1;
			}
		}

	} else { // Custom font

		// Character is assumed previously filtered by write() to eliminate
		// newlines, returns, non-printable characters, etc.  Calling drawChar()
		// directly with 'bad' characters of font may cause mayhem!

		c -= pgm_read_byte(&gfxFont->first);
		GFXglyph *glyph  = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
		uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);

		uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
		uint8_t  w  = pgm_read_byte(&glyph->width),
					 h  = pgm_read_byte(&glyph->height),
					 xa = pgm_read_byte(&glyph->xAdvance);
		int8_t   xo = pgm_read_byte(&glyph->xOffset),
					 yo = pgm_read_byte(&glyph->yOffset);
		uint8_t  xx, yy, bits, bit = 0;
		int16_t  xo16, yo16;

		if(size > 1) {
			xo16 = xo;
			yo16 = yo;
		}

		// Todo: Add character clipping here

		// NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
		// THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
		// has typically been used with the 'classic' font to overwrite old
		// screen contents with new data.  This ONLY works because the
		// characters are a uniform size; it's not a sensible thing to do with
		// proportionally-spaced fonts with glyphs of varying sizes (and that
		// may overlap).  To replace previously-drawn text when using a custom
		// font, use the getTextBounds() function to determine the smallest
		// rectangle encompassing a string, erase the area with fillRect(),
		// then draw new text.  This WILL infortunately 'blink' the text, but
		// is unavoidable.  Drawing 'background' pixels will NOT fix this,
		// only creates a new set of problems.  Have an idea to work around
		// this (a canvas object type for MCUs that can afford the RAM and
		// displays supporting setAddrWindow() and pushColors()), but haven't
		// implemented this yet.

		for(yy=0; yy<h; yy++) {
			for(xx=0; xx<w; xx++) {
				if(!(bit++ & 7)) {
					bits = pgm_read_byte(&bitmap[bo++]);
				}
				if(bits & 0x80) {
					if(size == 1) {
						drawPixel(x+xo+xx, y+yo+yy);
					} else {
						fillRect(x+(xo16+xx)*size, y+(yo16+yy)*size, size, size);
					}
				}
				bits <<= 1;
			}
		}

	} // End classic vs custom font
}
	*/