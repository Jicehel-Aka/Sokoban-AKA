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
#ifdef __cplusplus
extern "C" {
#endif


#include "gb_common.h"

#define GB_AUDIO_BUFFER_FIFO_COUNT      4   // count of audio buffer fifo
#define GB_AUDIO_BUFFER_SAMPLE_COUNT    512 // 16b audio buffers (size in bytes = 2*GB_AUDIO_BUFFER_SAMPLE_COUNT )

    //! initialise audio codec and I2S bus. @return GB_OK (ESP_ERROR_CHECK aborts on fatal I2S fail)
int gb_ll_audio_init();
    //! push audio buffer for playing
void gb_ll_audio_push_buffer( const int16_t* pi16_audio_buffer );
    //! return total count of buffer 
uint32_t gb_ll_audio_fifo_buffer_count();
    //! return count of buffers used
uint32_t gb_ll_audio_fifo_buffer_used();
    //! return count of free buffers
uint32_t gb_ll_audio_fifo_buffer_free();
    // 0 (mute) to 255 (max)
void gb_ll_audio_set_volume( uint8_t u8_volume );
    // toggle on/off vibrator
void gb_ll_audio_set_vibrator( uint8_t u8_on );


#ifdef __cplusplus
}
#endif