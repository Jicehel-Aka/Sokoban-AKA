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
#include "gb_common.h"
#pragma once

    //! base object for audio
class gb_audio_track_base {
    public:
    gb_audio_track_base() {};
     ~gb_audio_track_base() {};
        // implement of derivated class to generate audio buffer with requested sample count
    virtual int play_callback( int16_t* pi16_buffer, uint16_t u16_sample_count ) = 0;
        // implement of derivated class to stop playing
    virtual void stop_playing() = 0;
        // implement of derivated class to return true if playing currently in progress
    virtual bool is_playing( ) = 0;
        // implement of derivated class current position
    virtual uint32_t position();
        // implement of derivated class to return true if playing currently in progress
    virtual uint32_t length( );



        //! generate fade out effect on buffer ( volume down ramp )
    void fade_out_buffer( int16_t* pi16_buffer );
        //! generate fade in effect on buffer ( volume up ramp )
    void fade_in_buffer( int16_t* pi16_buffer );
        //! fix track volume ( 0.0 to 1.0 )
    void set_track_volume( float f32_volume );
        //! return current track volume ( 0.0 to 1.0 )
    float get_track_volume();

    protected:
            //! relative volume used by gb_audio_player for mix
        float _f32_volume{1.0f};
        int32_t i32_cb_loop_count{0};
};

#define AUDIO_PLAYER_TRACK_COUNT 4
class gb_audio_player  {
    public:
        gb_audio_player();
        ~gb_audio_player() {};
            //! add a track type to player
            //! @return GB_OK, GB_ERR_PARAM (null), GB_ERR_NO_SPACE (4 slots full)
        int add_track( gb_audio_track_base* track, float f32_volume = 1.0f );
            //! remove a track from player
            //! @return GB_OK, GB_ERR_PARAM (null), GB_ERR_NOT_FOUND
        int del_track( gb_audio_track_base* track );
            //! update track objects ( generate samples )
        void pool();
            // set voume to 0 (mute) to 255 (max)
        void set_master_volume( uint8_t u8_volume );
            // return current volume
        uint8_t get_master_volume();
            //! start vibrator for requested delay in milliseconds. call with 0 to stop 
        void vibrator( uint32_t u32_delay_ms );
    private:
        gb_audio_track_base* tracks[AUDIO_PLAYER_TRACK_COUNT];
        uint32_t u32_vibrator_start{0};
        uint32_t u32_vibrator_delay{0};
        uint8_t _u8_master_volume{0};
};
