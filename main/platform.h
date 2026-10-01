/*
 * platform.h - the few things that differ between the console (ESP-IDF) and the PC (SDL2).
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace plat {

void init();                                  // mounts nothing: the SD card is mounted by gb_core::init()
const char* data_dir();                       // folder SOKOBAN (levelpacks, sound, music)
const char* save_dir();                       // where CFG.DAT / PROGRESS.DAT / screenshots go
void data_path(char* out, size_t n, const char* sub);
void save_path(char* out, size_t n, const char* name);
void make_dir(const char* path);
bool file_exists(const char* path);

[[noreturn]] void return_to_loader();         // console: reboots into the launcher; PC: quits

// Tiny mutex, used between the game loop and the audio pump.
void* mutex_create();
void  mutex_lock(void* m);
void  mutex_unlock(void* m);

// Starts the loop that keeps the audio mixer fed (a FreeRTOS task on the console, the SDL audio
// callback on the PC). `pump` is called often; it must lock by itself.
void start_audio_pump(void (*pump)(void));

// Runs `fn` on a task with a big enough stack (console) or directly (PC).
void run_game(void (*fn)(void));

}  // namespace plat
