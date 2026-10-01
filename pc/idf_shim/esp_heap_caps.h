#pragma once
#include <stddef.h>
#define MALLOC_CAP_SPIRAM 1
#define MALLOC_CAP_8BIT   2
#define MALLOC_CAP_INTERNAL 4
static inline size_t heap_caps_get_largest_free_block(unsigned) { return 0; }
static inline size_t heap_caps_get_free_size(unsigned) { return 0; }
