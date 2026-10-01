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

gb_audio_track_fx_t gb_fx_system_ok[] = 
{
    { 4*110, 4*(110-6), 126/255.0, (126-11)/255.0, 200, gb_audio_track_tone::SINE },
    { 4*150, 4*(150-25), (47)/255.0, (47-3)/255.0, 300, gb_audio_track_tone::SINE }
};

gb_audio_track_fx_t gb_fx_system_ko[] = 
{
    { 4*120, 4*(120-3), 126/255.0, (126-8)/255.0, 200, gb_audio_track_tone::SINE },
    { 4*130, 4*(130-13), (169)/255.0, (169-10)/255.0, 300, gb_audio_track_tone::SINE }
};

gb_audio_track_fx_t gb_fx_system_tick[] = 
{
    { 4*196, 4*(196-35), 142/255.0, (142-3)/255.0, 100, gb_audio_track_tone::SINE },
};


gb_audio_track_fx_t gb_fx_system_explode[] = 
{
    { 220, 440, 0.0, 1.0, 250, gb_audio_track_tone::NOISE },
    { 440, 330, 1.0, 0.5, 500, gb_audio_track_tone::NOISE },
    { 330, 220, 0.5, 0.0, 250, gb_audio_track_tone::NOISE },
};