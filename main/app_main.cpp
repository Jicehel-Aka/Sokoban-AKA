/*
 * app_main.cpp - entry point. On the console ESP-IDF calls app_main(); on the PC main() calls it.
 *
 * Sokoban for Gamebuino AKA - a port of "Sokoban (GP2X)" by Willems Davy (joyrider3774), MIT.
 * SPDX-License-Identifier: MIT
 */
#include "gamebuino.h"

#include "app.h"
#include "platform.h"

// The three objects of the gamebuino library, shared by every module.
gb_core g_core;
gb_graphics g_gfx;
gb_audio_player g_audio;

extern "C" void app_main(void)
{
    plat::init();
    plat::run_game(app::run);
#ifndef AKA_PC
    // On the console this task has nothing more to do.
#endif
}
