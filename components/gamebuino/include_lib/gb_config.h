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


//#define USE_PSRAM_VIDEO_BUFFER  // alloc 320x240 video buffer in PSRAM, instead or static alloc in DRAM. Less performances, but more free SRAM

//#define USE_VIDEO_256_INDEXED   // use 256 colors video buffer, instead of 65536 colors. less RAM usage, and more preformance, but only 256 colors
#define USE_VSYCNC // sync to screen refresh : 70 / 35 .. fps or free FPS

#define ZERO_BUFFER_PSRAM   // 1 SRAM video buffer render directly from SRAM ( no double buffer )
//#define ONCE_BUFFER_PSRAM   // 1 PSRAM + 1 SRAM video buffer : copy SRAM to PSRAM buffer
//#define DUAL_BUFFER_PSRAM // 2 PSRAM video buffer, swap video pointers



/*
options video : 
    Single buffer 
        primary 16b en SRAM
        primary 16b en PSRAM
    double buffer
        primary 8b SRAM, secondary 16b PSRAM
        primary 16b SRAM, secondary 16b PSRAM
        primary 16b PSRAM, secondary 16b PSRAM

*/