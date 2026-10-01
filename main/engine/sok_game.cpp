/*
 * sok_game.cpp - see sok_game.h.
 * SPDX-License-Identifier: MIT
 */
#include "sok_game.h"

#include <string.h>

namespace sok {

void Game::start(const Board& b)
{
    initial_ = b;
    cur_ = b;
    n_ = top_ = 0;
    moves_ = pushes_ = 0;
    facing_ = DIR_DOWN;
}

void Game::restart()
{
    start(initial_);
}

bool Game::all_goals_covered() const
{
    return cur_.goals > 0 && cur_.boxes_on_goals() >= (int)cur_.goals;
}

void Game::apply_forward(Dir d, bool pushed)
{
    const int dx = dir_dx(d), dy = dir_dy(d);
    const int nx = cur_.px + dx, ny = cur_.py + dy;
    if (pushed) {
        cur_.cell[ny][nx] &= (uint8_t)~F_BOX;
        cur_.cell[ny + dy][nx + dx] |= F_BOX;
    }
    cur_.px = (uint8_t)nx;
    cur_.py = (uint8_t)ny;
    facing_ = d;
}

void Game::apply_backward(Dir d, bool pushed)
{
    const int dx = dir_dx(d), dy = dir_dy(d);
    if (pushed) {
        // The box now stands two tiles ahead of the old player position, i.e. one ahead of the player.
        cur_.cell[cur_.py + dy][cur_.px + dx] &= (uint8_t)~F_BOX;
        cur_.cell[cur_.py][cur_.px] |= F_BOX;
    }
    cur_.px = (uint8_t)(cur_.px - dx);
    cur_.py = (uint8_t)(cur_.py - dy);
    facing_ = d;
}

bool Game::move(Dir d, Step* out)
{
    facing_ = d;                       // the sprite turns even when the move is blocked
    const int dx = dir_dx(d), dy = dir_dy(d);
    const int nx = cur_.px + dx, ny = cur_.py + dy;
    if (cur_.is_wall(nx, ny)) return false;

    bool pushed = false;
    if (cur_.is_box(nx, ny)) {
        const int bx = nx + dx, by = ny + dy;
        if (cur_.is_wall(bx, by) || cur_.is_box(bx, by)) return false;
        pushed = true;
    }

    if (n_ == MAX_HISTORY) {           // history full: forget the oldest step
        memmove(hist_, hist_ + 1, MAX_HISTORY - 1);
        --n_;
    }
    hist_[n_++] = (uint8_t)(d | (pushed ? 4 : 0));
    top_ = n_;                         // a new move discards the redo branch

    apply_forward(d, pushed);
    ++moves_;
    if (pushed) ++pushes_;
    if (out) { out->dir = d; out->pushed = pushed; }
    return true;
}

bool Game::undo(Step* out)
{
    if (n_ == 0) return false;
    const uint8_t e = hist_[--n_];
    const Dir d = (Dir)(e & 3);
    const bool pushed = (e & 4) != 0;
    apply_backward(d, pushed);
    --moves_;
    if (pushed) --pushes_;
    if (out) { out->dir = d; out->pushed = pushed; }
    return true;
}

bool Game::redo(Step* out)
{
    if (n_ >= top_) return false;
    const uint8_t e = hist_[n_++];
    const Dir d = (Dir)(e & 3);
    const bool pushed = (e & 4) != 0;
    apply_forward(d, pushed);
    ++moves_;
    if (pushed) ++pushes_;
    if (out) { out->dir = d; out->pushed = pushed; }
    return true;
}

}  // namespace sok
