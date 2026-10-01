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

#ifdef __cplusplus
extern "C" {
#endif

#include "gb_common.h"

    //! Drive expander reset/power lines after I2C is up. @return GB_OK
int gb_ll_expander_init();

void gb_ll_expander_lcd_reset(uint8_t state);
#if (BOARD_VERSION < 4) // hard connected on V1.4+
void gb_ll_expander_lcd_cs(uint8_t state);
#else
void gb_ll_expander_lcd_rd(uint8_t state);
#endif
void gb_ll_expander_audio_amplifier_reset(uint8_t state);
void gb_ll_expander_power_off();

#ifdef __cplusplus
}
#endif