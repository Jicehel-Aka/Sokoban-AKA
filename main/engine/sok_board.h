/*
 * sok_board.h - Sokoban board model and level parser (hardware independent).
 *
 * Part of Sokoban-AKA, a port of "Sokoban (GP2X)" by Willems Davy (joyrider3774)
 * to the Gamebuino AKA console. Original: https://github.com/joyrider3774/Sokoban
 * Original (c) 2006-2026 Willems Davy, MIT License - see LICENSE and CREDITS.md.
 * This file is a new implementation of the same game rules.
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace sok {

// Same playfield limits as the original game (26 x 15 tiles).
constexpr int MAX_W = 26;
constexpr int MAX_H = 15;

// Cell flags.
enum : uint8_t {
    F_WALL   = 1,   // wall
    F_GOAL   = 2,   // storage spot
    F_BOX    = 4,   // a box stands here
    F_INSIDE = 8,   // floor reachable from the player's start (what gets drawn as floor)
};

enum Dir : uint8_t { DIR_RIGHT = 0, DIR_DOWN = 1, DIR_LEFT = 2, DIR_UP = 3 };

inline int dir_dx(Dir d) { return d == DIR_RIGHT ? 1 : (d == DIR_LEFT ? -1 : 0); }
inline int dir_dy(Dir d) { return d == DIR_DOWN ? 1 : (d == DIR_UP ? -1 : 0); }
inline Dir dir_opposite(Dir d) { return (Dir)((d + 2) & 3); }

enum class ParseStatus : uint8_t {
    Ok = 0,
    Empty,          // no wall found
    TooBig,         // bigger than MAX_W x MAX_H
    NoPlayer,
    ManyPlayers,
    NoBox,
    NoGoal,
    BoxGoalMismatch // fewer boxes than goals: could never be solved
};

struct Board {
    uint8_t  w = 0, h = 0;                 // used area (top-left normalised to 0,0)
    uint8_t  cell[MAX_H][MAX_W] = {};
    uint8_t  px = 0, py = 0;               // player position
    uint16_t boxes = 0, goals = 0;

    bool in_range(int x, int y) const { return x >= 0 && y >= 0 && x < w && y < h; }
    bool is_wall(int x, int y) const { return !in_range(x, y) || (cell[y][x] & F_WALL); }
    bool is_box(int x, int y) const { return in_range(x, y) && (cell[y][x] & F_BOX); }
    bool is_goal(int x, int y) const { return in_range(x, y) && (cell[y][x] & F_GOAL); }
    int  boxes_on_goals() const;

    // Flags the floor reachable from the player (walls stop the flood fill).
    void compute_inside();
};

// True if `c` can appear in a Sokoban board row.
bool is_board_char(char c);

// Parses one board. `text` holds the rows separated by '\n' (an optional '\r' is ignored).
// Characters: '#' wall, '@' player, '+' player on goal, '$' box, '*' box on goal,
// '.' goal, ' ' '-' '_' floor. The result is trimmed to the bounding box of its content.
ParseStatus parse_board(const char* text, size_t len, Board& out);

const char* parse_status_name(ParseStatus s);

}  // namespace sok
