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
#include "gb_audio_track_pmf.h"
#include "stdio.h"


    //! initialize pmf player from pmf data : data must stay  accessible after return
void gb_audio_track_pmf::load_pmf( const void *pmem_pmf_file )
{
    _pmf_player.load( pmem_pmf_file );
}

    //! sytart playing previous loaded data 
void gb_audio_track_pmf::play_pmf()
{
    _pmf_player.start(GB_AUDIO_SAMPLE_RATE);
}
    //! load and start playing sound from pmf data : data must stay  accessible after return
void gb_audio_track_pmf::play_pmf( const void *pmem_pmf_file )
{
    _pmf_player.load( pmem_pmf_file );
    _pmf_player.start(GB_AUDIO_SAMPLE_RATE);
}


            // return playing state
bool gb_audio_track_pmf::is_playing()
{
    return _pmf_player.is_playing();
}


        // stop sound
void gb_audio_track_pmf::stop_playing()
{
    _pmf_player.stop();
}



        // implement of derivated class current position
uint32_t gb_audio_track_pmf::position()
{
    return _pmf_player.playlist_pos();
}
        // implement of derivated class to return true if playing currently in progress
uint32_t gb_audio_track_pmf::length()
{
    return _pmf_player.playlist_length();
}

//pmf_audio_buffer<int16_t, GB_AUDIO_BUFFER_SAMPLE_COUNT> s_pmf_audio_buffer;


        //! buffer fill callback 
int gb_audio_track_pmf::play_callback( int16_t* pi16_buffer, uint16_t u16_sample_count )
{
    
    if ( _pmf_player.is_playing() )
    {
        memset( pi16_buffer, 0, 2*u16_sample_count );
        _pmf_player.mix(pi16_buffer, u16_sample_count);
        return 0; // playing

    }
    return -1; // not playing
}

    //////////////////////////// pmf_player platform implementation ///////////////////////////


uint32_t pmf_player::get_sampling_freq(uint32_t sampling_freq_) const
{
  // Tu peux éventuellement forcer à 44100 / 32000 / 22050
  // selon ta config I2S. Si ton moteur tourne déjà à sample_rate fixe,
  // tu peux simplement retourner sampling_freq_.
  return GB_AUDIO_SAMPLE_RATE;
//  return sampling_freq_;
}

void pmf_player::start_playback(uint32_t sampling_freq_)
{
    // nothing to do, gb_audio_player do all needed
  (void)sampling_freq_;
}


void pmf_player::stop_playback()
{
    // nothing to do, gb_audio_player do all needed
}

pmf_mixer_buffer pmf_player::get_mixer_buffer()
{
  // On n'utilise pas update()+get_mixer_buffer dans ce backend,
  // car on passe par pmf_player::mix().
  pmf_mixer_buffer buf = { 0, 0 };
  return buf;
}



void pmf_player::mix_buffer(pmf_mixer_buffer &buf_, unsigned num_samples_)
{
  mix_buffer_impl<int16_t, false, 16>(buf_, num_samples_);
}


