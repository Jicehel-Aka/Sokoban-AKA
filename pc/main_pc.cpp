/*
 * main_pc.cpp - entry point of the PC (SDL2) build: the console's app_main() is
 * simply called from main(), exactly like ESP-IDF does after boot.
 * SPDX-License-Identifier: MIT
 */
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "pc_backend.h"

extern "C" void app_main(void);

int main(int argc, char** argv)
{
    SDL_SetMainReady();
    aka_pc_configure(argc, argv);
    app_main();
    return 0;
}
