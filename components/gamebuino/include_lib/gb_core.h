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
#include <stddef.h>
#include "gb_common.h"

#pragma once

class gb_buttons {
    public:
        enum gb_key { KEY_RUN=GB_KEY_RUN, KEY_MENU=GB_KEY_MENU,
             KEY_L1=GB_KEY_L1, KEY_R1=GB_KEY_R1,
             KEY_RIGHT=GB_KEY_RIGHT, KEY_UP=GB_KEY_UP, KEY_DOWN=GB_KEY_DOWN, KEY_LEFT=GB_KEY_LEFT, 
             KEY_A=GB_KEY_A, KEY_B=GB_KEY_B, KEY_C=GB_KEY_C, KEY_D=GB_KEY_D};
            //! update status of buttons
        void update();
            //! return buttons state, cf gb_buttons::KEY_xxx
        uint16_t state();
            //! return buttons press event, cf gb_buttons::KEY_xxx
        uint16_t pressed();
            //! return true if requested button pressed, cf gb_buttons::KEY_xxx
        bool pressed(gb_key key);
            //! return buttons release event, cf gb_buttons::KEY_xxx
        uint16_t released();
            //! return true if requested button released, cf gb_buttons::KEY_xxx
        bool released(gb_key key);
            //! If true, holding RUN in update() calls power_off(). Default false
            //! because games (pAKAman) use RUN for pause. Use gb_core::power_down()
            //! for an explicit shutdown.
        void set_run_power_off(bool enable);

    private:
        uint16_t u16_buttons {0};
        uint16_t u16_buttons_last {0};
        bool run_power_off {false};

};


class gb_joystick {
    public:
            //! update status of joystick
        void update();
            //! Joystick vertical position, from -1000 to 1000
        int16_t get_y();
            //! Joystick horizontal position, from -1000 to 1000
        int16_t get_x();

            //! D-Pad emulation on joystick : return buttons state, cf gb_buttons::KEY_xxx
        uint16_t state();
            //! D-Pad emulation on joystick : return buttons press event, cf gb_buttons::KEY_xxx
        uint16_t pressed();
            //! D-Pad emulation on joystick : return true if requested button presse, cf gb_buttons::KEY_xxx
        bool pressed(gb_buttons::gb_key key);
            //! D-Pad emulation on joystick : return buttons release event, cf gb_buttons::KEY_xxx
        uint16_t released();
            //! D-Pad emulation on joystick : return true if requested button released, cf gb_buttons::KEY_xxx
        bool released(gb_buttons::gb_key key);

            //! Joystick horizontal integral position, like a mouse
        float get_posx();
            //! Joystick vertical integral position, like a mouse
        float get_posy();
            //! calibrage center position
        void calibrate_center();
            //! set X Joystick position range, like a mouse. Set fmin=0 and fmax=0 to disable feature ( default )
        void set_posx_range( float fmin, float fmax );
            //! set Y Joystick position range, like a mouse. Set fmin=0 and fmax=0 to disable feature ( default )
        void set_posy_range( float fmin, float fmax );
            //! set XY Joystick integral dispacment speed in pix/seconds ( default = 500 )
        void set_posxy_speed( float fspeed );
            //! set XY Joystick integral position ( default = center of screen )
        void set_posxy( float posx, float posy );

    private:
        int16_t i16_joy_x {0}; int16_t i16_joy_y {0};
        int32_t i32_joy_center_x {JOYX_MID};
        int32_t i32_joy_center_y {JOYX_MID};
        float f32_joy_pos_x{160}; float f32_joy_pos_x_min{0}; float f32_joy_pos_x_max{319};
        float f32_joy_pos_y{120}; float f32_joy_pos_y_min{0}; float f32_joy_pos_y_max{240};
        float f32_joy_speed_xy{500.0}; // joy speed, default : up to 500 pix/sec
        uint32_t u32_last_update{0}; // last update date in ms
            // DPad emulation
        uint16_t u16_buttons {0};
        uint16_t u16_buttons_last {0};

};


class gb_core {
    public:
        gb_core();
        ~gb_core();
            //! initialise gamebuino : screen, bus and peripherals.
            //! @return GB_OK if I2C/ADC/audio inits succeeded. SD failure is
            //!         logged but does not fail init (game can run without card).
            //!         LCD has no return code (void driver).
        int init();
            //! update status of buttons, joystick...
        void pool();
            //! buton management
        gb_buttons buttons;
            //! Jystick management
        gb_joystick joystick;
            //! delay in milliseconds
        void delay_ms(uint32_t u32_ms);
            //! return date in milliseconds
        uint32_t get_millis();
            //! return date in microseconds
        int64_t get_micros();
            //! return count of free PSRAM
        size_t free_psram();
            //! return count of free SRAM
        size_t free_sram();
            //! power down 
        void power_down();    
    private:
};




