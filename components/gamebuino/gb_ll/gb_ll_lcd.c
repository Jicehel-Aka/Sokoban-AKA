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
#include "gb_config.h"
#include <esp_lcd_panel_io.h>
#include "esp_lcd_io_i80.h"
#include "gb_ll_lcd.h"
#include "gb_ll_expander.h"
#include <string.h> // memcpy
#include "gb_ll_system.h"


static volatile uint32_t u32_start_refresh = 0;
static volatile uint32_t u32_delta_refresh = 0;
static volatile uint8_t  u8_refresh_ctr = 0;
static volatile uint32_t u32_draw_count = 0;

IRAM_ATTR bool color_trans_done_cb(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) 
{
    u32_delta_refresh = gb_get_millis() - u32_start_refresh;
    u8_refresh_ctr=1;
    u32_start_refresh = 0;
    return false;
}

/**
 * @brief LCD clock source
 * @note User should select the clock source based on the real requirement:
 * @verbatim embed:rst:leading-asterisk
 * +---------------------+-------------------------+----------------------------+
 * | LCD clock source    | Features                | Power Management           |
 * +=====================+=========================+============================+
 * | LCD_CLK_SRC_PLL160M | High resolution         | ESP_PM_APB_FREQ_MAX lock   |
 * +---------------------+-------------------------+----------------------------+
 * | LCD_CLK_SRC_PLL240M | High resolution         | ESP_PM_APB_FREQ_MAX lock   |
 * +---------------------+-------------------------+----------------------------+
 * | LCD_CLK_SRC_APLL    | Configurable resolution | ESP_PM_NO_LIGHT_SLEEP lock |
 * +---------------------+-------------------------+----------------------------+
 * | LCD_CLK_SRC_XTAL    | Medium resolution       | No PM lock                 |
 * +---------------------+-------------------------+----------------------------+
 * @endverbatim
 */
esp_lcd_i80_bus_handle_t i80_bus = NULL;
esp_lcd_i80_bus_config_t bus_config = {
    .dc_gpio_num = LCD_PIN_DnC,
    .wr_gpio_num = LCD_PIN_nWR,
    .clk_src = LCD_CLK_SRC_PLL160M, // 7.885 ms
//    .clk_src = LCD_CLK_SRC_PLL240M, // 7.885 ms
    .data_gpio_nums = {
        LCD_PIN_DB0,
        LCD_PIN_DB1,
        LCD_PIN_DB2,
        LCD_PIN_DB3,
        LCD_PIN_DB4,
        LCD_PIN_DB5,
        LCD_PIN_DB6,
        LCD_PIN_DB7,
    },
    .bus_width = 8,
    .max_transfer_bytes = 320 * 240 * sizeof(uint16_t), // transfer 100 lines of pixels (assume pixel is RGB565) at most in one transaction
//    .max_transfer_bytes = 320 * 100 * sizeof(uint16_t), // transfer 100 lines of pixels (assume pixel is RGB565) at most in one transaction
    .psram_trans_align = 64,
    .sram_trans_align = 64,
};

esp_lcd_panel_io_i80_config_t panel_config = {
  .cs_gpio_num = -1,
    // 320x240*2*50 Hz = 8MHz Min : target < 20 ms
//  .pclk_hz = 80000000, // 1 ms 1000 fps, FAIL colors
//  .pclk_hz = 40000000, //  5 ms / 200 fps, FAIL colors
//  .pclk_hz = 32000000, //  5 ms / 200 fps, FAIL colors
//  .pclk_hz = 24000000, //  7 ms / 140 fps, FAIL colors
  .pclk_hz = 20000000, //  8 ms / 125 fps, OK ILI 2.6 TFT + ST 2.8
//  .pclk_hz = 16000000, // 10 ms / 100 fps, OK ( 70 fps screen )
//  .pclk_hz = 8000000,  // 20 ms / 50 fps, OK
//  .pclk_hz = 4000000,  // 39 ms / 25 fps, OK
//  .pclk_hz = 2000000,  // 77 ms / 12 fps, OK
  
//  .trans_queue_depth = 320*240,
    .trans_queue_depth = 320*16,
    .on_color_trans_done = color_trans_done_cb,
    .user_ctx = 0,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8,
    .dc_levels = {
        .dc_idle_level = 1,
        .dc_cmd_level = 0,
        .dc_dummy_level = 1,
        .dc_data_level = 1
    },
    .flags = {
        .cs_active_high = 0,
        .reverse_color_bits = 0,
        .swap_color_bytes = 1,
        .pclk_active_neg = 0,
        .pclk_idle_low = 0
    }
};
esp_lcd_panel_io_handle_t lcd_panel_h;
#include "hal/lcd_types.h"
#include "esp_lcd_panel_st7789.h"
#include "esp_lcd_io_i80.h"

void LCD_i80_bus_init()
{
    printf( "esp_lcd_new_i80_bus\n");
    esp_err_t ret;
    ret = esp_lcd_new_i80_bus( &bus_config, &i80_bus );
    printf( "esp_lcd_new_i80_bus return %d\n", ret );
    if  (ret )
        for(;;)
    gb_delay_ms(100);

    printf( "esp_lcd_new_panel_io_i80\n");
//    ESP_ERROR_CHECK( ret=esp_lcd_new_panel_io_i80(i80_bus, &panel_config, &lcd_panel_h) );
    ret = esp_lcd_new_panel_io_i80(i80_bus, &panel_config, &lcd_panel_h);
    printf( "esp_lcd_new_panel_io_i80 return %d\n", ret );
    if  (ret )
        for(;;)
    gb_delay_ms(100);
}

void LCD_i80_bus_uninit()
{
    esp_lcd_panel_io_del(lcd_panel_h);
    esp_lcd_del_i80_bus(i80_bus);
}

    // hw reset
void ILI9342C_hard_reset()
{
    gb_ll_expander_lcd_reset(0);
    gb_delay_ms(10);
    gb_ll_expander_lcd_reset(1);
    gb_delay_ms(100);
#if (BOARD_VERSION < 4) // CD# hard wired on v1.4+
    expander_lcd_cs(0);
#endif    
    gb_delay_ms(100);
}


#include "esp_lcd_panel_ops.h"
#include "hal/gpio_hal.h"
#include "driver/gpio.h"
#define LOW 0
#define HIGH 1
#define INPUT 1
#define OUTPUT 0

esp_err_t gpio_func_sel(gpio_num_t gpio_num, uint32_t func);
void pinMode( gpio_num_t pin, uint8_t u8_level )
{
    gpio_func_sel( pin , PIN_FUNC_GPIO);
    if ( u8_level == OUTPUT )
    {
        gpio_set_direction(pin, GPIO_MODE_OUTPUT );
        gpio_input_enable(pin);
    }
    else
    {
        gpio_set_direction(pin, GPIO_MODE_INPUT );
    }
}

void digitalWrite( gpio_num_t pin, uint8_t u8_level )
{
    gpio_ll_set_level(&GPIO, pin, u8_level);
}

int digitalRead( gpio_num_t pin )
{
    return gpio_get_level(pin);
}

void LCD_FAST_test(const gb_pixel* u16_pframe_buffer)
{
    u32_start_refresh = gb_get_millis(); // start date
//    ESP_ERROR_CHECK( esp_lcd_panel_io_tx_color ( lcd_panel_h, ST7789V_CMD_RAMWR, u16_pframe_buffer, 320*240*2 ) );
    esp_lcd_panel_io_tx_color ( lcd_panel_h, ST7789V_CMD_RAMWR, u16_pframe_buffer, SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(gb_pixel_16) );
}

void ILI9342C_write_cmd( uint8_t cmd, const uint8_t *pu8_data, int len )
{
    esp_lcd_panel_io_tx_param ( lcd_panel_h, cmd, pu8_data, len );
}

uint32_t LCD_last_refresh_delay()
{
    return u32_delta_refresh;
}

const uint8_t gamebuinoLogo[] = {80,10,
	0b00111100,0b00111111,0b00111111,0b11110011,0b11110011,0b11110011,0b00110011,0b00111111,0b00111111,0b00011100,
	0b00111100,0b00111111,0b00111111,0b11110011,0b11110011,0b11110011,0b00110011,0b00111111,0b00111111,0b00100110,
	0b00110000,0b00110011,0b00110011,0b00110011,0b00000011,0b00110011,0b00110011,0b00110011,0b00110011,0b00100110,
	0b00110000,0b00110011,0b00110011,0b00110011,0b00000011,0b00110011,0b00110011,0b00110011,0b00110011,0b00101010,
	0b00110011,0b00111111,0b00110011,0b00110011,0b11110011,0b11000011,0b00110011,0b00110011,0b00110011,0b00011100,
	0b00110011,0b00111111,0b00110011,0b00110011,0b11110011,0b11000011,0b00110011,0b00110011,0b00110011,0b00000000,
	0b00110011,0b00110011,0b00110011,0b00110011,0b00000011,0b00110011,0b00110011,0b00110011,0b00110011,0b00000000,
	0b00110011,0b00110011,0b00110011,0b00110011,0b00000011,0b00110011,0b00110011,0b00110011,0b00110011,0b00000000,
	0b00111111,0b00110011,0b00110011,0b00110011,0b11110011,0b11110011,0b11110011,0b00110011,0b00111111,0b00000000,
	0b00111111,0b00110011,0b00110011,0b00110011,0b11110011,0b11110011,0b11110011,0b00110011,0b00111111,0b00000000,
};

void lcd_drawBitmap( int16_t x, int16_t y, const uint8_t *bitmap, uint16_t u16_pix_color )
{
	uint8_t w = *(bitmap++);
	uint8_t h = *(bitmap++);
	
	uint8_t byteWidth = (w + 7) / 8;
	uint8_t _x = x;
	uint8_t dw = 8 - (w%8);
	for (uint8_t j = 0; j < h; j++) {
		x = _x;
		for (uint8_t i = 0; i < byteWidth;) {
			uint8_t b = *(bitmap++);
			i++;
			for (uint8_t k = 0; k < 8; k++) {
				if (i == byteWidth && k == dw) {
					x += (w%8);
					break;
				}
				if (b&0x80) {
					lcd_putpixel(x, y, u16_pix_color);
				}
				b <<= 1;
				x++;
			}
		}
		y++;
	}
}

void lcd_clear( uint16_t u16_pix_color )
{
    gb_pixel* pdest = &framebuffer[0];
    uint32_t u32_count = SCREEN_WIDTH*SCREEN_HEIGHT/4;
    while(u32_count--) {
        *pdest++ = u16_pix_color;
        *pdest++ = u16_pix_color;
        *pdest++ = u16_pix_color;
        *pdest++ = u16_pix_color;
    }
}

void lcd_dpo()
{
    #ifndef USE_VIDEO_256_INDEXED
    gb_pixel* pdest = &framebuffer[0];
    uint32_t u32_count = SCREEN_WIDTH*SCREEN_HEIGHT;
    while(u32_count--) {
        gb_pixel c = *pdest;
        c &= 0xF7DF;
        *pdest++ = c>>1;
    }
    #endif // of not defined USE_VIDEO_256_INDEXED
}


// ST7789V_MADCTRL_MV
void st7789v_rotation_set(uint8_t rotation)
{
	static uint8_t mad_cfg = 0;
    switch (rotation % 4) {
        case 0:
        	mad_cfg = ST7789V_MADCTRL_MX | ST7789V_MADCTRL_MY | ST7789V_MADCTRL_RGB;
            //Column address (MX): Right to left
            //Page address (MY): Bottom to top
            //Page/ Column order (MV): normal
            //RGB/BGR order: RGB
            break;
        case 1:
        	mad_cfg = ST7789V_MADCTRL_MV | ST7789V_MADCTRL_RGB;
            //Column address (MX): Left to right
            //Page address (MY): Top to bottom
            //Page/ Column order (MV): reverse
            //RGB/BGR order: RGB
            break;
        case 2:
        	mad_cfg = ST7789V_MADCTRL_RGB;
            //Column address (MX): Left to right
            //Page address (MY): Top to bottom
            //Page/ Column order (MV): normal
            //RGB/BGR order: RGB
            break;
        case 3:
        	mad_cfg = ST7789V_MADCTRL_MX | ST7789V_MADCTRL_MV | ST7789V_MADCTRL_RGB;
            //Column address (MX): Right to left
            //Page address (MY): Top to bottom
            //Page/ Column order (MV): reverse
            //RGB/BGR order: RGB
            break;
        default:
            break;
    } 
    ILI9342C_write_cmd( ST7789V_CMD_MADCTL, &mad_cfg, 1);
}


void set_addr_window(uint16_t x_0, uint16_t y_0, uint16_t x_1, uint16_t y_1)
{
    // copy coordinates
    static uint8_t x_coord[4];
    static uint8_t y_coord[4];
    x_coord[0] = x_0 >> 8;
    x_coord[1] = (uint8_t) x_0;
    x_coord[2] = x_1 >> 8;
    x_coord[3] = (uint8_t) x_1;
    y_coord[0] = y_0 >> 8;
    y_coord[1] = (uint8_t) y_0;
    y_coord[2] = y_1 >> 8;
    y_coord[3] = (uint8_t) y_1;
    ILI9342C_write_cmd( ST7789V_CMD_CASET, x_coord, 4 );
    ILI9342C_write_cmd( ST7789V_CMD_RASET, y_coord, 4 );
}


    //! set hard fps to @u8_fps, from 40 to 100
void lcd_set_fps( uint8_t u8_fps )
{
    if ( u8_fps > 100 )
        u8_fps = 100; // bound max available  fps
    if ( u8_fps < 40 )
        u8_fps = 40; // bound min available fps
    uint32_t u32_div = 10000000;
    u32_div = u32_div/(336*u8_fps);
    if ( u32_div > 250 )
        u32_div -= 250;
    else 
        u32_div = 0; 
    uint8_t u8_fr_cfg = u32_div/16;
    if ( u8_fr_cfg > 31 ) // 5 bits value
        u8_fr_cfg = 31;
    if ( u8_fr_cfg < 1 )
        u8_fr_cfg = 1;
    printf( "Sef FPS to %d : %d\n", u8_fps, u8_fr_cfg );
    ILI9342C_write_cmd( 0xC6, &u8_fr_cfg, 1);
}


#ifdef USE_PSRAM_VIDEO_BUFFER
    gb_pixel* framebuffer;
#else
    gb_pixel framebuffer[SCREEN_WIDTH*SCREEN_HEIGHT];  // static in SRAM (rendrer)
#endif


#include "driver/ledc.h"


#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          PWM_LCD_GPIO // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
//#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY          (PWM_LCD_FREQUENCY) // Frequency in Hertz. Set frequency at 40 kHz

/* Warning:
 * For ESP32, ESP32S2, ESP32S3, ESP32C3, ESP32C2, ESP32C6, ESP32H2, ESP32P4 targets,
 * when LEDC_DUTY_RES selects the maximum duty resolution (i.e. value equal to SOC_LEDC_TIMER_BIT_WIDTH),
 * 100% duty cycle is not reachable (duty cannot be set to (2 ** SOC_LEDC_TIMER_BIT_WIDTH)).
 */

static void lcd_init_pwm()
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 40 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}



void lcd_update_pwm(uint16_t u16_duty)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, u16_duty));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

void gb_ll_lcd_init()
{
    lcd_init_pwm();
    lcd_update_pwm(64);

    #ifdef USE_PSRAM_VIDEO_BUFFER
        framebuffer = (gb_pixel*)heap_caps_calloc( 2*320*240, sizeof(gb_pixel), MALLOC_CAP_SPIRAM  );
        printf( "Alloc ram ptr %p\n",  framebuffer );
        printf("Print memory caps after alloc\n");
        heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
        while (!framebuffer)
        {
            printf( "Alloc ram FAIL\n" );
            delay(1000);
        }
    #endif

    pinMode( LCD_FMARK, INPUT);
    gb_ll_expander_lcd_rd(1);
//    uint8_t u8_scanline = 0;
    printf( "LCD_i80_bus_init()\n" );
    LCD_i80_bus_init();

    ILI9342C_hard_reset();
    ILI9342C_write_cmd( ST7789V_CMD_RESET, 0, 0 );
    ILI9342C_write_cmd( ST7789V_CMD_SLPOUT, 0, 0 );
    const uint8_t color_mode[] = {COLOR_MODE_65K | COLOR_MODE_16BIT};
    ILI9342C_write_cmd( ST7789V_CMD_COLMOD, color_mode, 1);
    ILI9342C_write_cmd( ST7789V_CMD_INVON, 0, 0);
    ILI9342C_write_cmd( ST7789V_CMD_NORON, 0, 0);
    st7789v_rotation_set(3);
    set_addr_window(0, 0, 319, 239);
    ILI9342C_write_cmd( ST7789V_CMD_DISPON, 0, 0);
    const uint8_t te_cfg[] = { 0x00 }; // Tearing effecct output mode : V-Blanking information only
    ILI9342C_write_cmd( 0x35, te_cfg, 1);

    u8_refresh_ctr = 1; // start with end of refresh set


    lcd_set_fps(100); // max for intro scrool
    for ( int16_t y = -10 ; y < (240-10)/2 ; y+=4 )
    {
//        lcd_clear(color_blue);
        lcd_clear(color_black);
        lcd_drawBitmap( (320-80)/2, y, gamebuinoLogo, color_orange );
        lcd_refresh();
        while( !lcd_refresh_completed() ); // for simple buffer : wait refresh completed before erase screen buffer
    }
    lcd_set_fps(100); // default fps
}


void lcd_putpixel( uint16_t x, uint16_t y, gb_pixel u16_color )
{
    uint32_t u32_offset = x + y*SCREEN_WIDTH;
    if ( u32_offset < SCREEN_WIDTH*SCREEN_HEIGHT )  
        framebuffer[u32_offset] = u16_color;
}

gb_pixel lcd_getpixel( uint16_t x, uint16_t y)
{
    uint32_t u32_offset = x + y*SCREEN_WIDTH;
    if ( u32_offset < SCREEN_WIDTH*SCREEN_HEIGHT )  
        return framebuffer[u32_offset];
    return 0; // default
}

uint8_t lcd_refresh_completed()
{
    return u8_refresh_ctr;
}



    //! return count of lcd draw since power on
uint32_t gb_ll_lcd_get_draw_count()
{
    return u32_draw_count;
}


void lcd_refresh()
{
    while( u8_refresh_ctr == 0 );  // wait last update compleped ( DMA )
    u8_refresh_ctr = 0;            // reset complete dma flag
    #ifdef USE_VSYCNC
      // Sync to Scanline
    uint64_t start_us = gb_get_micros();
    while ( digitalRead( LCD_FMARK ) )
    {
        if ( ( gb_get_micros() - start_us ) > 20000 )
        {
            printf( "ERROR : Loop scanline 1 timeout\n" );
            break;
        }
    }
//    printf( "Loop scanline 1 %lld us\n", esp_timer_get_time() - start_us );

    start_us = gb_get_micros();
    while ( digitalRead( LCD_FMARK ) == 0 )
    {
        if ( ( gb_get_micros() - start_us ) > 20000 )
        {
            printf( "ERROR : Loop scanline 0 timeout\n" );
            break;
        }
    }
//    printf( "Loop scanline 0 %lld us\n", esp_timer_get_time() - start_us );
    #endif


    u32_draw_count++;
    LCD_FAST_test(framebuffer);     // start DMA update
}


void lcd_scrool_vertical( int16_t scrool_pixels )
{
    if ( scrool_pixels > 0 ) // scrool up
    {
        scrool_pixels =  MIN( scrool_pixels, SCREEN_HEIGHT-1 );
        memcpy( framebuffer, &framebuffer[SCREEN_WIDTH*scrool_pixels], 
            SCREEN_WIDTH*(SCREEN_HEIGHT-scrool_pixels)*sizeof(gb_pixel) );
        memset( &framebuffer[SCREEN_WIDTH*(SCREEN_HEIGHT-scrool_pixels)], 0, SCREEN_WIDTH*scrool_pixels*sizeof(gb_pixel) );
    }
    // TODO : implement scrool down ( caution : overlap area : copy from bottom )

}



