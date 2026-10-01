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
#include <stdint.h>

    //! initialize 3 ADC channels (battery + joystick). @return GB_OK or GB_ERR_IO
int gb_ll_adc_init();
    //! return Battery voltage in mv
int gb_ll_adc_read_vbatt_mv();
    //! return battery voltage as % : 0 ~ 100
int gb_ll_adc_read_vbatt_percent();
    //! return joystick X coordinate as mv (0 .. 3300)
int gb_ll_adc_read_joyx();
    //! return joystick Y coordinate as mv (0 .. 3300)
int gb_ll_adc_read_joyy();

#ifdef __cplusplus
}
#endif