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
#include "gb_audio_track_fx.h"
#include "stdio.h"

void gb_audio_track_fx::play_fx( const gb_audio_track_fx_t* fx_array, size_t fx_count)
{
    if ( is_playing() ) // allow abort current
        stop_playing();
    _fx_array = fx_array;
    _fx_count = fx_count;
    _fx_current_index = 0;

}
    //! buffer fill callback 
int gb_audio_track_fx::play_callback( int16_t* pi16_buffer, uint16_t u16_sample_count )
{
    if ( !track_tone.is_playing() )
    {
        if ( _fx_current_index < _fx_count )
        {
            track_tone.play_tone( 
                        _fx_array[_fx_current_index].f32_start_frequency,
                        _fx_array[_fx_current_index].f32_end_frequency,
                        _fx_array[_fx_current_index].f32_start_volume,
                        _fx_array[_fx_current_index].f32_end_volume,
                        _fx_array[_fx_current_index].u16_duration_ms,
                        _fx_array[_fx_current_index].type );
            _fx_current_index++;
        }
        else
        {
            stop_playing();
            return -1; // not playing
        }
    }

    if ( track_tone.is_playing() )
    {
        track_tone.play_callback( pi16_buffer, u16_sample_count );
        return 0; // playing
    }
    return -1; // not playing
}

    // stop sound
void gb_audio_track_fx::stop_playing()
{
    _fx_array = 0;
    _fx_count = 0;
    track_tone.stop_playing();
}

        // return playing state
bool gb_audio_track_fx::is_playing()
{
    if ( _fx_count )
        return true;
    return false;    
}
