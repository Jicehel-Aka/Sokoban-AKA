/*
 * sok_edit.h - model of the level editor: an editable character grid, its conversion
 * to/from a Board, and the writer of ".sok" files (sokobano.de text format).
 *
 * Part of Sokoban-AKA, a port of "Sokoban (GP2X)" by Willems Davy (joyrider3774) (MIT).
 * The original game has a level editor too; this is a new, hardware independent
 * implementation (it is unit tested on the PC).
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "sok_board.h"

namespace sok {

enum class Part : uint8_t { Floor, Wall, Goal, Box, Player };

struct Grid {
    // ' ' floor/empty, '#' wall, '$' box, '.' goal, '*' box on goal, '@' player, '+' player on goal
    char c[MAX_H][MAX_W];

    Grid() { clear(); }
    void clear();
    bool empty() const;

    // Puts a part on a cell. Floor erases. Goal/Box/Player combine with what is there
    // (a box on a goal is '*'); a wall replaces everything; there is only one player.
    void place(int x, int y, Part p);

    bool from_board(const Board& b);

    // Text of the used area (bounding box), rows separated by '\n', no trailing newline.
    // Returns the length, or 0 when the grid is empty / `n` is too small.
    size_t to_text(char* out, size_t n) const;

    // Same checks as when a pack is loaded: exactly one player, boxes >= goals >= 1, a wall...
    ParseStatus validate(Board* out = nullptr) const;
};

// Writes a pack: "Set:" / "Author:" header, then every level followed by "Title: Level N".
// Writes to "<path>.tmp" then renames, so a failure never destroys an existing file.
bool write_sok(const char* path, const char* set_name, const char* author, const Grid* levels, int count);

}  // namespace sok
