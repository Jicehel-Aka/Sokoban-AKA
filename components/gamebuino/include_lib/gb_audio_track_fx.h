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
#include <stddef.h>
#include "gb_audio_player.h"
#include "gb_audio_track_tone.h"

#pragma once


typedef struct {
    float f32_start_frequency;
    float f32_end_frequency;
    float f32_start_volume;
    float f32_end_volume;
    uint16_t u16_duration_ms;
    gb_audio_track_tone::tone_type type;
} gb_audio_track_fx_t;


//! base object for audio
class gb_audio_track_fx : public gb_audio_track_base {
    public:
        //! start playing sound for duration(ms) and frequency(Hz)
    void play_fx( const gb_audio_track_fx_t* fx_array, size_t fx_count );
        //! buffer fill callback 
    int play_callback( int16_t* pi16_buffer, uint16_t u16_sample_count );
        // stop sound
    void stop_playing();
            // return playing state
    bool is_playing();

    private:
        gb_audio_track_tone track_tone;
        const gb_audio_track_fx_t* _fx_array {0};
        size_t _fx_count{0};
        size_t _fx_current_index{0};

};