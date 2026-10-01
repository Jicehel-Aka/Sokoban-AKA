/*
 * sok_board.cpp - see sok_board.h.
 * SPDX-License-Identifier: MIT
 */
#include "sok_board.h"

#include <string.h>

namespace sok {

bool is_board_char(char c)
{
    switch (c) {
        case '#': case '@': case '+': case '$': case '*': case '.':
        case ' ': case '-': case '_':
            return true;
        default:
            return false;
    }
}

int Board::boxes_on_goals() const
{
    int n = 0;
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            if ((cell[y][x] & (F_BOX | F_GOAL)) == (F_BOX | F_GOAL))
                ++n;
    return n;
}

void Board::compute_inside()
{
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            cell[y][x] &= (uint8_t)~F_INSIDE;

    // Iterative flood fill with an explicit stack: no recursion on a 4 KB task stack.
    static_assert(MAX_W * MAX_H < 65536, "stack index must fit in uint16_t");
    uint16_t stack[MAX_W * MAX_H];
    int sp = 0;
    if (!in_range(px, py) || (cell[py][px] & F_WALL))
        return;
    cell[py][px] |= F_INSIDE;
    stack[sp++] = (uint16_t)(py * MAX_W + px);
    while (sp > 0) {
        const uint16_t v = stack[--sp];
        const int x = v % MAX_W, y = v / MAX_W;
        static const int8_t dx[4] = {1, -1, 0, 0}, dy[4] = {0, 0, 1, -1};
        for (int k = 0; k < 4; ++k) {
            const int nx = x + dx[k], ny = y + dy[k];
            if (!in_range(nx, ny)) continue;
            uint8_t& c = cell[ny][nx];
            if (c & (F_WALL | F_INSIDE)) continue;
            c |= F_INSIDE;
            stack[sp++] = (uint16_t)(ny * MAX_W + nx);
        }
    }
}

ParseStatus parse_board(const char* text, size_t len, Board& out)
{
    out = Board();

    // Pass 1: bounding box of everything that is not floor.
    int min_x = 1 << 20, min_y = 1 << 20, max_x = -1, max_y = -1;
    int row = 0, col = 0;
    for (size_t i = 0; i <= len; ++i) {
        const char c = (i < len) ? text[i] : '\n';
        if (c == '\n') { ++row; col = 0; continue; }
        if (c == '\r') continue;
        if (c != ' ' && c != '-' && c != '_') {
            if (col < min_x) min_x = col;
            if (col > max_x) max_x = col;
            if ((row) < min_y) min_y = row;
            if (row > max_y) max_y = row;
        }
        ++col;
    }
    if (max_x < 0) return ParseStatus::Empty;

    const int w = max_x - min_x + 1, h = max_y - min_y + 1;
    if (w > MAX_W || h > MAX_H) return ParseStatus::TooBig;
    out.w = (uint8_t)w;
    out.h = (uint8_t)h;

    // Pass 2: fill the cells.
    int players = 0;
    row = 0; col = 0;
    for (size_t i = 0; i <= len; ++i) {
        const char c = (i < len) ? text[i] : '\n';
        if (c == '\n') { ++row; col = 0; continue; }
        if (c == '\r') continue;
        const int x = col - min_x, y = row - min_y;
        ++col;
        if (x < 0 || y < 0 || x >= w || y >= h) continue;
        uint8_t& cell = out.cell[y][x];
        switch (c) {
            case '#': cell |= F_WALL; break;
            case '$': cell |= F_BOX; ++out.boxes; break;
            case '*': cell |= F_BOX | F_GOAL; ++out.boxes; ++out.goals; break;
            case '.': cell |= F_GOAL; ++out.goals; break;
            case '@': out.px = (uint8_t)x; out.py = (uint8_t)y; ++players; break;
            case '+': cell |= F_GOAL; ++out.goals; out.px = (uint8_t)x; out.py = (uint8_t)y; ++players; break;
            default: break;
        }
    }
    if (players == 0) return ParseStatus::NoPlayer;
    if (players > 1) return ParseStatus::ManyPlayers;
    if (out.boxes == 0) return ParseStatus::NoBox;
    if (out.goals == 0) return ParseStatus::NoGoal;
    if (out.boxes < out.goals) return ParseStatus::BoxGoalMismatch;
    out.compute_inside();
    return ParseStatus::Ok;
}

const char* parse_status_name(ParseStatus s)
{
    switch (s) {
        case ParseStatus::Ok: return "ok";
        case ParseStatus::Empty: return "empty";
        case ParseStatus::TooBig: return "too big";
        case ParseStatus::NoPlayer: return "no player";
        case ParseStatus::ManyPlayers: return "several players";
        case ParseStatus::NoBox: return "no box";
        case ParseStatus::NoGoal: return "no goal";
        case ParseStatus::BoxGoalMismatch: return "fewer boxes than goals";
    }
    return "?";
}

}  // namespace sok
