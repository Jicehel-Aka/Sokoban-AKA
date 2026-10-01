/* Minimal stand-in for the few ESP-IDF symbols the gamebuino library headers need
 * when it is compiled on a desktop PC (SDL2 backend). Not used on the console. */
#pragma once
#include <stdint.h>
#define IRAM_ATTR
#define DRAM_ATTR
#define EXT_RAM_ATTR
typedef uint32_t TickType_t;
typedef int BaseType_t;
#define pdMS_TO_TICKS(x) ((TickType_t)(x))
#define portTICK_PERIOD_MS 1
#define portMAX_DELAY 0xffffffffu
typedef int gpio_num_t;
