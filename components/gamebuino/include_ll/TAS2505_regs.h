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
// TI TAS2505 audio controler

#define AUDIO_AMP_REG_PAGE      0x00
    // Page 0 ( AUDIO_AMP_REG_PAGE = 0 )
#define AUDIO_AMP_SOFT_RESET        0x01    // Software Reset Register
#define AUDIO_AMP_P0_CLK_SETTING1   0x04    // Clock Setting Register 1, Multiplexers
#define AUDIO_AMP_P0_CLK_SETTING2   0x05    // Clock Setting Register 2, PLL P and R Values
#define AUDIO_AMP_AIS_REG1          0x1B    // Audio Interface Setting Register 1
#define AUDIO_AMP_P0_GPIO_CTRL      0x34    // GPIO control

    // Page 1 ( AUDIO_AMP_REG_PAGE = 1 )
#define AUDIO_AMP_P1_LDO_CTRL       0x02    // LDO Control Register
#define AUDIO_AMP_P1_SPK_AMP        0x2D    //Speaker Amplifier Control 1
#define AUDIO_AMP_P1_SPK_VOL        0x2E    // Speaker Volume Control Register
#define AUDIO_AMP_P1_SPK_AMP_VOL    0x30    // Speaker Amplifier Volume Control 2
#define AUDIO_AMP_P1_HP_SPK_VOL        0x16    // HP  Volume Control Register
