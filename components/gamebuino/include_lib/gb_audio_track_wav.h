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
#include "gb_err.h"
#include "esp_vfs_fat.h"

#pragma once

//! the WAV file format header
typedef struct {
	uint32_t ChunkID;				//!< 'RIFF'
	uint32_t FileSize;				//!< in bytes
	uint32_t FileFormat;			//!< 'WAVE'

	uint32_t SubChunk1ID;			//!< 'fmt '
	uint32_t SubChunk1Size;			//!< 16
	uint16_t AudioFormat;			//!< 1
	uint16_t NbrChannels;   		//!< 1, 2 = mono/stereo
	uint32_t SampleRate;    		//!< 22050, 44100
	uint32_t ByteRate;				//!< foo
	uint16_t BlockAlign;			//!< always 4
	uint16_t BitPerSample;  		//!< always 16
	uint32_t SubChunk2ID;			//!< identify the samples 'data'
	uint32_t SubChunk2DataSize;		//!< the wav data size ( header not included )
} WAVE_FormatTypeDef;

//! base object for audio
class gb_audio_track_wav : public gb_audio_track_base {
    public:
        //! start playing WAV from file on SD. Path must be under MOUNT_POINT, no "..".
        //! @return GB_OK, GB_ERR_PARAM, GB_ERR_NOT_MOUNTED, GB_ERR_IO, GB_ERR_FORMAT, GB_ERR_OVERFLOW
    int play_wav( const char* pc_file_name );
        //! start playing WAV from ESP memory. Pointer must remain valid until stop.
        //! @return GB_OK, GB_ERR_PARAM, GB_ERR_FORMAT, GB_ERR_OVERFLOW
    int play_wav( const uint8_t * pi8_wav_file );
	        //! start playing raw 16-bit samples from RAM. @return GB_OK or GB_ERR_PARAM
	int play_raw( const int16_t* pi16_sample, size_t sample_count );
        //! buffer fill callback. @return GB_OK if playing, GB_ERR if idle
    int play_callback( int16_t* pi16_buffer, uint16_t u16_sample_count );
	        //! export WAV file to a C header on SD. Same path rules as play_wav.
	int convert_wav_to_header( const char* pc_file_name_in, const char* pc_file_name_out );
        // stop sound
    void stop_playing();
        // return playing state
    bool is_playing();

    private:
			// check if file format is correct ( 44k, mono, 16b )
		int check_file_format(  WAVE_FormatTypeDef* pinfo );
        FILE* f_wav {0};
		const int16_t* p_sample {0}; 
        uint32_t u32_sample_count {0};	// count of 16b samples
        uint32_t u32_sample_index {0};	// current pos in 16b sample count
        WAVE_FormatTypeDef WAVE_info;
};
