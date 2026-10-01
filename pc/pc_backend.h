/*
 * pc_backend.h - what the desktop (SDL2) backend offers to the game on top of the
 * Gamebuino-AKA low level replacement. Only used when building for PC.
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Parses the command line (--scale N, --fullscreen, --data DIR, --save DIR, --help). Call first.
void aka_pc_configure(int argc, char** argv);

// Directory holding the program (always ends without a slash). Used to find the SOKOBAN data folder.
const char* aka_pc_exe_dir(void);
// Overrides given on the command line / environment (NULL when absent).
const char* aka_pc_data_dir_override(void);
const char* aka_pc_save_dir_override(void);

// Audio: the SDL audio thread calls `fn` before it consumes samples, so that the game's
// gb_audio_player::pool() runs at the pace of the sound card (on the console a FreeRTOS
// task plays that role). `fn` must do its own locking.
void aka_pc_set_audio_pump(void (*fn)(void));

// Small mutex wrapper so game code does not need SDL headers.
void* aka_pc_mutex_create(void);
void  aka_pc_mutex_lock(void* m);
void  aka_pc_mutex_unlock(void* m);

// True when the game runs from a script (tests): virtual clock, no real time pacing.
int aka_pc_scripted(void);

#ifdef __cplusplus
}
#endif
