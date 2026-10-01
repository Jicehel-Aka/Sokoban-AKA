// foo file for compatibility

#include <stdint.h>
#include <string.h>

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const uint8_t*)(addr))
#endif

#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const uint16_t*)(addr))
#endif

#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const uint32_t*)(addr))
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

    // for macro MAX(), MIN()
#include "../include_lib/gb_common.h"
