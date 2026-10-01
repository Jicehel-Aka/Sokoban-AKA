/*
 * save.h - settings (CFG.DAT) and level progress (PROGRESS.DAT).
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdint.h>

namespace save {

struct Config {
    uint8_t lang = 0;          // i18n::Lang
    uint8_t music = 8;         // 0..10
    uint8_t sfx = 9;           // 0..10
    uint8_t smooth = 1;        // animate moves
    uint8_t unlock_all = 0;    // every level of a pack selectable
    char    last_pack[40] = "";
    uint16_t last_level = 0;
};

bool load_config(Config& c);
bool save_config(const Config& c);

// Progress per pack: number of unlocked levels (1 = only the first), best moves per level are not kept.
int  unlocked(const char* pack, int level_count);              // always in [1, level_count]
void set_unlocked(const char* pack, int count);               // only ever grows

}  // namespace save
