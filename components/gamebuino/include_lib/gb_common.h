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
#include <stdint.h>
#include "gb_config.h"
#include "gb_ll_common.h"
#include "gb_err.h"
#pragma once
/* BOARD_VERSION is defined once in gb_ll_common.h (included above). */


typedef uint16_t    gb_pixel_16;// 16 bits color
typedef uint8_t     gb_pixel_8; // 8 bits indexed color

#define SCREEN_WIDTH            320
#define SCREEN_HEIGHT           240
#define GB_AUDIO_SAMPLE_RATE    44100   //<! high quality, more ressource
//#define GB_AUDIO_SAMPLE_RATE    22050   //<! medium quality, medium ressource
//#define GB_AUDIO_SAMPLE_RATE    11025   //<! low quality, low ressource
//#define GB_AUDIO_SAMPLE_RATE    96000   //<! very high quality, more ressource : We need to get our butts in gear.

#define MOUNT_POINT             "/sdcard" // base path for files on SDcard


#ifdef USE_VIDEO_256_INDEXED
    typedef gb_pixel_8 gb_pixel;
#else
    typedef gb_pixel_16 gb_pixel;
#endif

#ifdef USE_PSRAM_VIDEO_BUFFER
    extern gb_pixel* framebuffer;
#else
    extern gb_pixel framebuffer[SCREEN_WIDTH*SCREEN_HEIGHT];  // static in SRAM (rendrer)
#endif

//#define USE_WIFI

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef BOUND
#define BOUND(a, mini, maxi) MIN(MAX(a,mini),maxi)
#endif

#define count_of(A) (sizeof(A)/sizeof(A[0]))

#define GB_KEY_RUN      EXPANDER_KEY_RUN
#define GB_KEY_MENU     EXPANDER_KEY_MENU
#define GB_KEY_L1   	EXPANDER_KEY_L1   
#define GB_KEY_R1   	EXPANDER_KEY_R1   
#define GB_KEY_RIGHT	EXPANDER_KEY_RIGHT
#define GB_KEY_UP   	EXPANDER_KEY_UP   
#define GB_KEY_DOWN 	EXPANDER_KEY_DOWN 
#define GB_KEY_LEFT 	EXPANDER_KEY_LEFT 
#define GB_KEY_A    	EXPANDER_KEY_A    
#define GB_KEY_B    	EXPANDER_KEY_B    
#define GB_KEY_C    	EXPANDER_KEY_C    
#define GB_KEY_D    	EXPANDER_KEY_D    


