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
#include <stdint.h>
#include "gb_ll_i2c.h"
#include "gb_common.h"
#include "gb_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "TAS2505_regs.h"
#include "gb_ll_system.h"


void gb_ll_expander_lcd_reset(uint8_t state)
{
    if ( state )
        gb_ll_expander_write( u8_expander_out_data | EXPANDER_LCD_nRESET );
    else    
        gb_ll_expander_write( u8_expander_out_data & ~EXPANDER_LCD_nRESET );
}


void gb_ll_expander_audio_amplifier_reset(uint8_t state)
{
    if ( state )
        gb_ll_expander_write( u8_expander_out_data | EXPANDER_AMP_nRESET );
    else    
        gb_ll_expander_write( u8_expander_out_data & ~EXPANDER_AMP_nRESET );
}
#if (BOARD_VERSION < 4) // hard connected on V1.4+
void expander_lcd_cs(uint8_t state)
{
    if ( state )
        expander_write( u8_expander_out_data | EXPANDER_LCD_nCS );
    else    
        expander_write( u8_expander_out_data & ~EXPANDER_LCD_nCS );
}
#else
void gb_ll_expander_lcd_rd(uint8_t state)
{
    if ( state )
        gb_ll_expander_write( u8_expander_out_data | EXPANDER_LCD_nRD );
    else    
        gb_ll_expander_write( u8_expander_out_data & ~EXPANDER_LCD_nRD );
}
#endif


void gb_ll_expander_power_off()
{
    while ( gb_ll_expander_read() & EXPANDER_KEY_RUN )
        ; // wait user release button to assure don't restart if user keep RUN key down after power off
    gb_ll_expander_write( u8_expander_out_data &= ~EXPANDER_AMP_nRESET );
    gb_ll_expander_write( u8_expander_out_data |= EXPANDER_KEY_RUN );
    gb_delay_ms(250);
    gb_ll_expander_write( u8_expander_out_data &= ~EXPANDER_OUT_ENA_3V3 );
}




int gb_ll_expander_init()
{
    #if (BOARD_VERSION < 4) // RD# en I2C expander
    expander_write( EXPANDER_OUT_ENA_3V3 | EXPANDER_AMP_nRESET | EXPANDER_LCD_nRESET | EXPANDER_LCD_nCS );
    #else
    gb_ll_expander_write( EXPANDER_OUT_ENA_3V3 | EXPANDER_AMP_nRESET | EXPANDER_LCD_nRESET | EXPANDER_LCD_nRD );
    #endif
    gb_ll_expander_write( EXPANDER_OUT_ENA_3V3   );
    gb_delay_ms(100);
    gb_ll_expander_write( EXPANDER_OUT_ENA_3V3 | EXPANDER_LCD_nRESET  );
    gb_delay_ms(100);
    return GB_OK;
}

