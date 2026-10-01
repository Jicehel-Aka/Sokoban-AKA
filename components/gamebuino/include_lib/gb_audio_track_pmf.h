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
#include "../gb_lib/pmf_player/pmf_player.h"
#include "gb_ll_audio.h"

#pragma once

//! base object for audio
class gb_audio_track_pmf : public gb_audio_track_base {
    public:
            //! initialize pmf player from pmf data : data must stay  accessible after return
        void load_pmf( const void *pmem_pmf_file );
            //! sytart playing previous loaded data 
        void play_pmf();
            //! load and start playing sound from pmf data : data must stay  accessible after return
        void play_pmf( const void *pmem_pmf_file );

            //! buffer fill callback 
        int play_callback( int16_t* pi16_buffer, uint16_t u16_sample_count );
            // stop sound
        void stop_playing();
            // implement of derivated class current position
        uint32_t position() override;
            // implement of derivated class to return true if playing currently in progress
        uint32_t length( ) override;
                // return playing state
        bool is_playing();

            // pmf specifics
            // return count of playback channels according current pfm file.
        uint8_t get_pmf_channels_count() {
            return _pmf_player.num_playback_channels();
        }
            // return track info for requested channel 
        pmf_channel_info get_pmf_channel_info(uint8_t channel_idx) {
            return _pmf_player.channel_info(channel_idx);
        }

    private:
        pmf_player _pmf_player;
};