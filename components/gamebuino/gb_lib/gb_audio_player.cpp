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

#include "gb_audio_player.h"
#include "gb_ll_audio.h"
#include "gb_ll_system.h"
#include "gb_err.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


gb_audio_player::gb_audio_player()
{
    for ( int i = 0 ; i < count_of(tracks) ; i++ )
        tracks[i] = 0;
}
  

    //! dfault : not implemented
uint32_t gb_audio_track_base::position()
{
    return 0;
}

    //! dfault : not implemented
uint32_t gb_audio_track_base::length()
{
    return 0;
}

//! generate fade out effect on buffer ( volume down ramp )
void gb_audio_track_base::fade_out_buffer( int16_t* pi16_buffer )
{
    for ( int16_t i = 0 ; i < GB_AUDIO_BUFFER_SAMPLE_COUNT ; i++ )
        pi16_buffer[i] = pi16_buffer[i] * (GB_AUDIO_BUFFER_SAMPLE_COUNT-1-i) / GB_AUDIO_BUFFER_SAMPLE_COUNT;
}
//! generate fade in effect on buffer ( volume up ramp )
void gb_audio_track_base::fade_in_buffer( int16_t* pi16_buffer )
{
    for ( int16_t i = 0 ; i < GB_AUDIO_BUFFER_SAMPLE_COUNT ; i++ )
        pi16_buffer[i] = pi16_buffer[i] * i / GB_AUDIO_BUFFER_SAMPLE_COUNT;
}


    //! fix a track volume ( 0.0 to 1.0 )
void gb_audio_track_base::set_track_volume( float f32_volume )
{
     _f32_volume = BOUND(f32_volume, 0.0f, 1.0f);
}

            //! return current track volume ( 0.0 to 1.0 )
float gb_audio_track_base::get_track_volume()
{
    return _f32_volume;
}



int gb_audio_player::add_track( gb_audio_track_base* track, float f32_volume )
{
    if(!track)
        return GB_ERR_PARAM;
    del_track(track); // only one instance of each track
    for ( int i = 0 ; i < count_of(tracks) ; i++ )
    {
        if ( tracks[i] == 0 )
        {
            tracks[i] = track;
            tracks[i]->set_track_volume(f32_volume);
            return GB_OK;
        }
    }
    return GB_ERR_NO_SPACE;
}

int gb_audio_player::del_track( gb_audio_track_base* track )
{
    if(!track)
        return GB_ERR_PARAM;
    for ( int i = 0 ; i < count_of(tracks) ; i++ )
    {
        if ( tracks[i] == track )
        {
            tracks[i] = 0;
            return GB_OK;
        }
    }
    return GB_ERR_NOT_FOUND;
}

void gb_audio_player::pool()
{
        // manage vibrator if on
    if ( u32_vibrator_delay )
    {
        if ( ( gb_get_millis() - u32_vibrator_start ) > u32_vibrator_delay )
        {
            gb_ll_audio_set_vibrator(0);
            u32_vibrator_delay = 0;
        }
    }
/*static int iPool = 0;
    if ( 0 == (iPool % 100) )
    {
        // caution : log on usbSerial cause small glitch on sound..
        printf("%ld count\n", audio_fifo_buffer_count() );
        printf("%ld used\n", audio_fifo_buffer_used() );
        printf("%ld free\n", audio_fifo_buffer_free() );
    }
    iPool++;
    */

    while ( gb_ll_audio_fifo_buffer_free() ) // at least 1 buffer
    {
        int8_t i8_buffer_ready = 0;
        static int16_t i16_audio_buffer_out[ GB_AUDIO_BUFFER_SAMPLE_COUNT];
        static int16_t i16_audio_buffer_track[ GB_AUDIO_BUFFER_SAMPLE_COUNT];
        for ( int smp = 0 ; smp < GB_AUDIO_BUFFER_SAMPLE_COUNT ; smp++ )
            i16_audio_buffer_out[smp] = 0;
        for ( int i = 0 ; i < count_of(tracks) ; i++ )
        {
            if ( tracks[i] ) // populated
            {
                if ( 0 == tracks[i]->play_callback(i16_audio_buffer_track, GB_AUDIO_BUFFER_SAMPLE_COUNT) ) // track generate successfully
                {
                    i8_buffer_ready++;
                    int32_t i32_track_volume = BOUND(tracks[i]->get_track_volume()*256, 0, 256);
                    for ( int smp = 0 ; smp < GB_AUDIO_BUFFER_SAMPLE_COUNT ; smp++ )
                        i16_audio_buffer_out[smp] += i32_track_volume*i16_audio_buffer_track[smp]/(256*count_of(tracks)/2);
                }
            }
        }
        if ( i8_buffer_ready )
            gb_ll_audio_push_buffer(i16_audio_buffer_out);
        if ( !i8_buffer_ready )
            return;
    }
}


void gb_audio_player::set_master_volume( uint8_t u8_volume )
{
    _u8_master_volume = u8_volume;
    gb_ll_audio_set_volume( u8_volume );
}

uint8_t gb_audio_player::get_master_volume()
{
    return _u8_master_volume;
}

void gb_audio_player::vibrator( uint32_t u32_delay_ms )
{
    u32_vibrator_start = gb_get_millis();
    u32_vibrator_delay = MIN(u32_delay_ms,1000); // up to 1 sec
    if ( u32_vibrator_delay )
        gb_ll_audio_set_vibrator(1);
    else
        gb_ll_audio_set_vibrator(0);
}



