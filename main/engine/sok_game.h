/*
 * sok_game.h - rules, move counters and undo/redo history of one Sokoban level.
 *
 * Part of Sokoban-AKA, a port of "Sokoban (GP2X)" by Willems Davy (joyrider3774)
 * (MIT). New implementation of the same rules: the player pushes one box at a
 * time, never pulls, and the level is solved when every goal holds a box.
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "sok_board.h"

namespace sok {

// Same history depth as the original game.
constexpr int MAX_HISTORY = 1000;

struct Step {
    Dir  dir = DIR_RIGHT;   // direction the player walked (for an undo: the direction undone)
    bool pushed = false;    // a box moved with the player
};

class Game {
public:
    // Loads a level and resets the counters and the history.
    void start(const Board& b);
    // Reloads the level the game was started with.
    void restart();

    // Tries to walk one tile. Returns false if blocked (wall, or a box that cannot move).
    bool move(Dir d, Step* out = nullptr);
    // Takes back / replays one step. Return false if nothing to undo / redo.
    bool undo(Step* out = nullptr);
    bool redo(Step* out = nullptr);

    const Board& board() const { return cur_; }
    int  moves() const { return moves_; }
    int  pushes() const { return pushes_; }
    bool can_undo() const { return n_ > 0; }
    bool can_redo() const { return n_ < top_; }
    Dir  facing() const { return facing_; }
    void set_facing(Dir d) { facing_ = d; }

    // Solved when every goal holds a box. Like the original game, a level that is
    // already solved at the start does not count: the player must have moved.
    bool solved() const { return moves_ > 0 && all_goals_covered(); }
    bool all_goals_covered() const;

private:
    void apply_forward(Dir d, bool pushed);
    void apply_backward(Dir d, bool pushed);

    Board   initial_;
    Board   cur_;
    uint8_t hist_[MAX_HISTORY] = {};   // bits 0-1: direction, bit 2: pushed
    int     n_ = 0;                    // undoable entries
    int     top_ = 0;                  // undoable + redoable entries
    int     moves_ = 0;
    int     pushes_ = 0;
    Dir     facing_ = DIR_DOWN;
};

}  // namespace sok
