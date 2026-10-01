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
#pragma once
#define BOARD_VERSION 4 // CORE V1.4


// setting PWM properties
#define PWM_LCD_GPIO            45
#define PWM_LCD_FREQUENCY       50000 // must be > 20000 to avoid being audible
#define PWM_LCD_LED_CHANNEL     0
#define PWM_LCD_RESOLUTION      8   /* LEDC duty is 0..255 (changelog 10-bit is outdated) */

#define SDCARD_PIN_CMD 38
#define SDCARD_PIN_CLK 2
#define SDCARD_PIN_D0 1
#define SDCARD_PIN_D1 15
#define SDCARD_PIN_D2 7
#define SDCARD_PIN_D3 16

//#define LCD_H_RESOLUTION  320
//#define LCD_V_RESOLUTION  240

#define LCD_H_RESOLUTION  240
#define LCD_V_RESOLUTION  320
#define LCD_FMARK      47

#define LCD_PIN_nWR    21
#define LCD_PIN_DnC   48
#define LCD_PIN_nRST   42 // connected to expander, mapped to CPU_MTMS => change this, use expander

#define LCD_PIN_DB7   14
#define LCD_PIN_DB6   13
#define LCD_PIN_DB5   12
#define LCD_PIN_DB4   11
#define LCD_PIN_DB3   10
#define LCD_PIN_DB2   9
#define LCD_PIN_DB1   46
#define LCD_PIN_DB0   3

#define I2S_PIN_BCLK  GPIO_NUM_18
#define I2S_PIN_WS    GPIO_NUM_17
#define I2S_PIN_DOUT  GPIO_NUM_8

#define NEOPIXELS_COUNT 4
#define NEOPIXELS_GPIO  44

// PCB V1.3
//#define EXPANDER_I2C_SCL   ((gpio_num_t)38)  // GPIO 38
//#define EXPANDER_I2C_SDA   ((gpio_num_t)37)  // GPIO 37
// PCB V1.4
#define EXPANDER_I2C_SCL   ((gpio_num_t)43)  // GPIO 43, shared with MCU_TX
#define EXPANDER_I2C_SDA   ((gpio_num_t)0)  // GPIO 0, shared with MCU_BOOT

#define EXPANDER_I2C_ADDRESS0   0x38 // 7 bits address
#define EXPANDER_I2C_ADDRESS1   0x3F // 7 bits address
#define AUDIO_AMP_I2C_ADDRESS   0x18 // 7 bits address

  // EXP0 : LOW byte
#define EXPANDER_OUT_ENA_3V3  0x0001
#define EXPANDER_KEY_RUN      0x0002
#define EXPANDER_KEY_MENU     0x0004
#define EXPANDER_AMP_nRESET   0x0008
#define EXPANDER_LCD_nRESET   0x0010
#define EXPANDER_LCD_nRD      0x0020
#define EXPANDER_KEY_L1       0x0080
#define EXPANDER_KEY_R1       0x0040
  // EXP1 : HIGH byte
#define EXPANDER_KEY_RIGHT    0x0100
#define EXPANDER_KEY_UP       0x0200
#define EXPANDER_KEY_DOWN     0x0400
#define EXPANDER_KEY_LEFT     0x0800
#define EXPANDER_KEY_A        0x8000
#define EXPANDER_KEY_B        0x2000
#define EXPANDER_KEY_C        0x4000
#define EXPANDER_KEY_D        0x1000

#define EXPANDER_KEY     ( EXPANDER_KEY_RUN | EXPANDER_KEY_MENU | EXPANDER_KEY_L1 | EXPANDER_KEY_R1 | EXPANDER_KEY_UP | EXPANDER_KEY_DOWN | EXPANDER_KEY_LEFT | EXPANDER_KEY_RIGHT | EXPANDER_KEY_A | EXPANDER_KEY_B | EXPANDER_KEY_C | EXPANDER_KEY_D )

#define ADC1_CHANNEL_BATTERY  3 // ADC1, channel 3
#if (BOARD_VERSION < 4) // hard connected on V1.4+
    #define ADC1_CHANNEL_JOYX     4
    #define ADC1_CHANNEL_JOYY     5
#else
    #define ADC1_CHANNEL_JOYX     5
    #define ADC1_CHANNEL_JOYY     4
#endif

#define JOYX_MAX    3300            // 3.300 V max
#define JOYX_LOW    (1*JOYX_MAX/4)  // 25% max : low level as DPAD
#define JOYX_MID    (2*JOYX_MAX/4)  // 50% max
#define JOYX_HIGH   (3*JOYX_MAX/4)  // 75% max : high level as DPAD



