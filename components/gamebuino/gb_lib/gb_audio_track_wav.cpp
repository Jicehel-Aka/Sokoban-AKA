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
#include "gb_audio_track_wav.h"
#include "esp_log.h"
#include "string.h"

int gb_audio_track_wav::check_file_format(  WAVE_FormatTypeDef* pinfo )
{
 #pragma GCC diagnostic ignored "-Wmultichar"

	if ( pinfo->ChunkID != 'FFIR' )
	{
		printf( "ERROR WAV file, invalid ChunkID" );
		return -1;
	}
	if ( pinfo->FileFormat != 'EVAW' )
	{
		printf( "ERROR WAV file, invalid FileFormat");
		return -1;
	}
	if ( pinfo->SubChunk1ID != ' tmf' )
	{
		printf( "ERROR WAV file, invalid SubChunk1ID" );
		return -1;
	}
	if ( pinfo->SubChunk1Size != 0x10 )
	{
		printf( "ERROR WAV file, invalid SubChunk1Size");
		return -1;
	}
	if ( pinfo->SubChunk2ID != 'atad' )
	{
		printf( "ERROR WAV file, invalid SubChunk2ID" );
		return -1;
	}
	if ( pinfo->NbrChannels != 1 )
	{
		printf( "ERROR file, invalid NbrChannels" );
		return -1;
	}
	if ( pinfo->BitPerSample != 16 )
	{
		printf( "ERROR WAV file, invalid BitPerSample" );
		return -1;
	}
	return 0;   
}

        //! start playing from ESP mem, samples only, without WAV file format
int gb_audio_track_wav::play_raw( const int16_t* pi16_sample, size_t sample_count )
{
    if ( is_playing() )
        stop_playing(); // allow to break song, and start new
    p_sample = pi16_sample;
    u32_sample_count = sample_count; 
    u32_sample_index = 0;
    return 0;
}


        //! start playing wav on scard
int gb_audio_track_wav::play_wav( const char* pc_file_name )
{
    if ( is_playing() )
        stop_playing(); // allow to break song, and start new
        
    printf( "Opening WAV %s\n", pc_file_name );
        // close if previous file opened

    f_wav = fopen(pc_file_name, "r");
    if ( f_wav == NULL )
    {
        printf( "Failed to open for reading\n");
        return ESP_FAIL;
    }

    if ( fread( &WAVE_info, sizeof(WAVE_info), 1, f_wav ) != 1 ) // 1 block of sizeof(WAVE_info)
    {
        printf( "Failed to read wav file %s for reading\n", pc_file_name );
        fclose(f_wav);
        f_wav = 0;
        return ESP_FAIL;
    }

    if ( check_file_format(&WAVE_info) ) 
    {
        printf( "Invalid wav file %s\n", pc_file_name );
        fclose(f_wav);
        f_wav = 0;
        return ESP_FAIL;
    }
    printf("WAV File %s : %ld bytes to read\n", pc_file_name, WAVE_info.SubChunk2DataSize );

    u32_sample_count = WAVE_info.SubChunk2DataSize/2; // convert data length to count of 16b samples
    u32_sample_index = 0;
    return 0;
}

    //! start playing WAV sound from ESP memory
int gb_audio_track_wav::play_wav( const uint8_t * pi8_wav_file )
{
    if ( is_playing() )
        stop_playing(); // allow to break song, and start new
    memcpy( &WAVE_info,  pi8_wav_file, sizeof(WAVE_info) );

    if ( check_file_format(&WAVE_info) ) 
    {
        printf( "Invalid memory wav format %p\n", pi8_wav_file );
        fclose(f_wav);
        f_wav = 0;
        return ESP_FAIL;
    }
    printf("WAV memory File %p : %ld bytes to read\n", pi8_wav_file, WAVE_info.SubChunk2DataSize );

    p_sample = (const int16_t*)&pi8_wav_file[sizeof(WAVE_info)]; // ptr just after WAV header
    u32_sample_count = WAVE_info.SubChunk2DataSize/2; // convert data length to count of 16b samples
    u32_sample_index = 0;
    return 0;        


}



        //! export WAV to header file
int gb_audio_track_wav::convert_wav_to_header( const char* pc_file_name_in, const char* pc_file_name_out )
{
    FILE* f_header = fopen(pc_file_name_out, "w");
    if ( f_header == NULL )
    {
        printf( "Failed to open output file for writing\n");
        return -1;
    }

        // open wav, and check format
    if ( play_wav(pc_file_name_in) )
        return -1; // error

    fprintf( f_header, "// %s\r\n", pc_file_name_in );
    fprintf( f_header, "const int16_t sound_name[] = {\r\n" );
    while ( u32_sample_index < u32_sample_count )
    {
        int16_t i16_sample;
        fread( &i16_sample, sizeof(i16_sample), 1, f_wav );    
        fprintf( f_header, "%d, ", i16_sample );
        u32_sample_index++;
    }
    fprintf( f_header, "\r\n};" );
    fclose(f_wav);
    fclose(f_header);
    return 0;

}


    // return playing state
bool gb_audio_track_wav::is_playing()
{
    if ( f_wav || p_sample )    // playing from wav file on sdcard or byte array on ESP memory
        return true;
    return false;    
}


void gb_audio_track_wav::stop_playing()
{
    if ( f_wav )
        fclose(f_wav);
    f_wav = 0;
    p_sample = 0;    
}

        //! buffer fill callback 
int gb_audio_track_wav::play_callback( int16_t* pi16_buffer, uint16_t u16_sample_count )
{
    if ( is_playing() )    // playing from wav file on sdcard or byte array on ESP memory
    {
        size_t u32_sample_to_read = (u32_sample_count - u32_sample_index);
        if ( u32_sample_to_read > u16_sample_count ) // 16b samples
            u32_sample_to_read = u16_sample_count;
        if ( f_wav )
            fread( pi16_buffer, 2*u32_sample_to_read, 1, f_wav );
        else
            memcpy( pi16_buffer, &p_sample[u32_sample_index], 2*u32_sample_to_read );    
//        printf("%ld/%ld\n", u32_sample_index, u32_sample_count);
        u32_sample_index += u32_sample_to_read;
        if ( u32_sample_index >= u32_sample_count )
            stop_playing();
        return 0; // success
    }

    return -1; // not playing
}

