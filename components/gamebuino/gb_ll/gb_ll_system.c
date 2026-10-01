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
#include "gb_ll_system.h"
#include "freertos/FreeRTOS.h"
#include <stdint.h>
#include "esp_timer.h"

void gb_delay_ms(uint32_t u32_ms)
{
    TickType_t ticks = pdMS_TO_TICKS(u32_ms);
    if (ticks < 1)
        ticks = 1;
    vTaskDelay(ticks);
}

IRAM_ATTR uint32_t gb_get_millis()
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

void gb_delay_us(int64_t u32_us)
{
    int64_t start_us = esp_timer_get_time();
    while ( (esp_timer_get_time() -start_us ) < u32_us )
        ;
}

IRAM_ATTR int64_t gb_get_micros()
{
    return esp_timer_get_time();
}

#include <esp_pm.h>

esp_pm_config_esp32s3_t pm_config = {
    .max_freq_mhz = 240,
    .min_freq_mhz = 80,
    .light_sleep_enable = true
};

void gb_ll_system_init()
{
    esp_timer_early_init();
    esp_pm_configure( &pm_config );
}

