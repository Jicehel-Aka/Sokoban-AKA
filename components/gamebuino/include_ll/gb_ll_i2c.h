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

extern uint8_t u8_expander_out_data;

/**
 * Initialize I2C master + expander chips + audio amp devices.
 * @return GB_OK or GB_ERR_IO
 */
int gb_ll_i2c_init();
    //! write 8b low expander (key input bits are forced HIGH). No-op if I2C not inited.
void gb_ll_expander_write(uint8_t u8_data);
    //! return 16b expander inputs (keys, cf EXPANDER_KEY_xxx). 0 if I2C read fails.
uint16_t gb_ll_expander_read();
    //! Write register on audio amplifier (logged on I2C error).
void gb_ll_audio_amp_write(uint8_t u8_reg, uint8_t u8_data);
    //! Read register from audio amplifier. Returns 0 on I2C error.
uint8_t gb_ll_audio_amp_read( uint8_t u8_reg );


#ifdef __cplusplus
}
#endif