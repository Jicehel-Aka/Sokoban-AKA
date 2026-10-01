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
#pragma once



//! base object for audio
class gb_audio_track_tone : public gb_audio_track_base {
    public:
    enum tone_type { SINE=0, SQUARE, TRIANGLE, NOISE };
        //! start playing sound for frequency(Hz), volume (0.0 to 1.0), and duration(ms) and 
    void play_tone( float f32_frequency, float f32_volume, uint16_t u16_duration_ms, tone_type type = SINE );
        //! start playing sound for duration(ms) and frequency(Hz) with effect on freq and volume. Note : volume range 0.0 to 1.0
    void play_tone( float f32_start_frequency, float f32_end_frequency, float f32_start_volume,  float f32_end_volume, uint16_t u16_duration_ms, tone_type fx_type = SINE );
        //! buffer fill callback 
    int play_callback( int16_t* pi16_buffer, uint16_t u16_sample_count );
        // stop sound
    void stop_playing();
            // return playing state
    bool is_playing();


    private:
            // sound con,figuration
        uint32_t u32_sin_inc_start{0};  // freq on begin of sound    
        uint32_t u32_sin_inc_end{0};    // freq on end of sound
        int32_t i32_volume_start{0};  // amplitude on begin of sound    
        int32_t i32_volume_end{0};    // amplitude on end of sound
        tone_type _fx_type{SINE};          // sound tpe
            // internal vars for calculus
        uint32_t u32_sin_sample_total{0};   // total 16b sample to generate
        uint32_t u32_sin_sample_index{0};   // current 16b sample index, from 0 to u32_sin_sample_total-1
        uint32_t u32_sin_read_index{0};     // current sine array read position
};