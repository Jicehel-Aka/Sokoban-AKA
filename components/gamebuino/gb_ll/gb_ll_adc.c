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
#include "gb_common.h"
#include "gb_err.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "soc/soc_caps.h"

    // ADC1 Calibration Init 
adc_cali_handle_t adc1_cali_chan0_handle = NULL;
adc_cali_handle_t adc1_cali_chan1_handle = NULL;
adc_cali_handle_t adc1_cali_chan2_handle = NULL;

adc_oneshot_unit_handle_t adc1_handle;
const adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
};

const adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
};


/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
const static char *TAG = "ADC";
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}


int gb_ll_adc_init()
{
    esp_err_t ret = adc_oneshot_new_unit(&init_config1, &adc1_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "adc_oneshot_new_unit failed %s -> GB_ERR_IO", esp_err_to_name(ret));
        return GB_ERR_IO;
    }

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_CHANNEL_BATTERY, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_CHANNEL_JOYX, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC1_CHANNEL_JOYY, &config));

    bool do_calibration1_chan0 = adc_calibration_init( ADC_UNIT_1, ADC1_CHANNEL_BATTERY, ADC_ATTEN_DB_12, &adc1_cali_chan0_handle);
    bool do_calibration1_chan1 = adc_calibration_init( ADC_UNIT_1, ADC1_CHANNEL_JOYX, ADC_ATTEN_DB_12, &adc1_cali_chan1_handle);
    bool do_calibration1_chan2 = adc_calibration_init( ADC_UNIT_1, ADC1_CHANNEL_JOYY, ADC_ATTEN_DB_12, &adc1_cali_chan2_handle);
    ESP_LOGI(TAG, "calibration batt=%d joyx=%d joyy=%d", do_calibration1_chan0, do_calibration1_chan1, do_calibration1_chan2);
    return GB_OK;
}

    // return battery voltage as mV : 2500 ~ 4200
int gb_ll_adc_read_vbatt_mv()
{
    int adc_raw, voltage;
    ESP_ERROR_CHECK( adc_oneshot_read(adc1_handle, ADC1_CHANNEL_BATTERY, &adc_raw) );
//    printf("RAW %d\n", adc_raw);
    adc_cali_raw_to_voltage( adc1_cali_chan0_handle, adc_raw, &voltage );
    return 2*voltage;
}

    // return battery voltage as % : 0 ~ 100
int gb_ll_adc_read_vbatt_percent()
{
    int adc_raw, voltage;
    ESP_ERROR_CHECK( adc_oneshot_read(adc1_handle, ADC1_CHANNEL_BATTERY, &adc_raw) );
    adc_cali_raw_to_voltage( adc1_cali_chan0_handle, adc_raw, &voltage );
    voltage*=2; // convert to battery voltage with 1:2 divider
    if ( voltage > 4000 )
        return 100; // full
    if ( voltage < 3300 )
        return 0; // empty

    return 100*(voltage - 3300)/(4000-3300);   
}

int gb_ll_adc_read_joyx()
{
    int adc_raw, voltage;
    ESP_ERROR_CHECK( adc_oneshot_read(adc1_handle, ADC1_CHANNEL_JOYX, &adc_raw) );
//    printf("RAW %d\n", adc_raw);
    adc_cali_raw_to_voltage( adc1_cali_chan1_handle, adc_raw, &voltage );
    return voltage;
}

int gb_ll_adc_read_joyy()
{
    int adc_raw, voltage;
    ESP_ERROR_CHECK( adc_oneshot_read(adc1_handle, ADC1_CHANNEL_JOYY, &adc_raw) );
//    printf("RAW %d\n", adc_raw);
    adc_cali_raw_to_voltage( adc1_cali_chan2_handle, adc_raw, &voltage );
    return voltage;
}
