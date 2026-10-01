/*
 * sok_edit.cpp - see sok_edit.h.
 * SPDX-License-Identifier: MIT
 */
#include "sok_edit.h"

#include <stdio.h>
#include <string.h>

namespace sok {

namespace {

struct Bits { bool wall, goal, box, player; };

Bits decode(char ch)
{
    Bits b = { false, false, false, false };
    switch (ch) {
        case '#': b.wall = true; break;
        case '.': b.goal = true; break;
        case '$': b.box = true; break;
        case '*': b.goal = b.box = true; break;
        case '@': b.player = true; break;
        case '+': b.goal = b.player = true; break;
        default: break;
    }
    return b;
}

char encode(const Bits& b)
{
    if (b.wall) return '#';
    if (b.goal && b.box) return '*';
    if (b.goal && b.player) return '+';
    if (b.box) return '$';
    if (b.goal) return '.';
    if (b.player) return '@';
    return ' ';
}

}  // namespace

void Grid::clear() { memset(c, ' ', sizeof c); }

bool Grid::empty() const
{
    for (int y = 0; y < MAX_H; ++y)
        for (int x = 0; x < MAX_W; ++x)
            if (c[y][x] != ' ') return false;
    return true;
}

void Grid::place(int x, int y, Part p)
{
    if (x < 0 || y < 0 || x >= MAX_W || y >= MAX_H) return;
    Bits b = decode(c[y][x]);
    switch (p) {
        case Part::Floor: b = Bits{ false, false, false, false }; break;
        case Part::Wall: b = Bits{ true, false, false, false }; break;
        case Part::Goal: b.wall = false; b.goal = true; break;
        case Part::Box: b.wall = false; b.box = true; b.player = false; break;
        case Part::Player:
            for (int j = 0; j < MAX_H; ++j)                        // a single player
                for (int i = 0; i < MAX_W; ++i) {
                    Bits o = decode(c[j][i]);
                    if (o.player) { o.player = false; c[j][i] = encode(o); }
                }
            b.wall = false; b.box = false; b.player = true;
            break;
    }
    c[y][x] = encode(b);
}

bool Grid::from_board(const Board& bd)
{
    clear();
    for (int y = 0; y < bd.h && y < MAX_H; ++y)
        for (int x = 0; x < bd.w && x < MAX_W; ++x) {
            const uint8_t f = bd.cell[y][x];
            Bits b = { (f & F_WALL) != 0, (f & F_GOAL) != 0, (f & F_BOX) != 0, false };
            if (x == bd.px && y == bd.py) b.player = true;
            c[y][x] = encode(b);
        }
    return true;
}

size_t Grid::to_text(char* out, size_t n) const
{
    int x0 = MAX_W, x1 = -1, y0 = MAX_H, y1 = -1;
    for (int y = 0; y < MAX_H; ++y)
        for (int x = 0; x < MAX_W; ++x)
            if (c[y][x] != ' ') {
                if (x < x0) x0 = x;
                if (x > x1) x1 = x;
                if (y < y0) y0 = y;
                if (y > y1) y1 = y;
            }
    if (x1 < 0) return 0;
    size_t o = 0;
    for (int y = y0; y <= y1; ++y) {
        int last = x1;
        while (last > x0 && c[y][last] == ' ') --last;               // trailing blanks are noise
        const int len = (c[y][last] == ' ') ? 0 : last - x0 + 1;
        if (o + (size_t)len + 2 > n) return 0;
        memcpy(out + o, &c[y][x0], (size_t)len);
        o += (size_t)len;
        if (y != y1) out[o++] = '\n';
    }
    out[o] = 0;
    return o;
}

ParseStatus Grid::validate(Board* out) const
{
    char text[MAX_W * MAX_H + MAX_H + 4];
    const size_t n = to_text(text, sizeof text);
    Board tmp;
    Board& b = out ? *out : tmp;
    return parse_board(text, n, b);
}

bool write_sok(const char* path, const char* set_name, const char* author, const Grid* levels, int count)
{
    char tmp[700];
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    FILE* f = fopen(tmp, "wb");
    if (!f) return false;
    bool ok = fprintf(f, "Set: %s\nAuthor: %s\n\n", set_name, author) > 0;
    char text[MAX_W * MAX_H + MAX_H + 4];
    for (int i = 0; i < count && ok; ++i) {
        if (levels[i].to_text(text, sizeof text) == 0) continue;     // never write an empty level
        ok = fprintf(f, "%s\nTitle: Level %d\n\n", text, i + 1) > 0;
    }
    if (fclose(f) != 0) ok = false;
    if (!ok) { remove(tmp); return false; }
    remove(path);                                                    // rename() cannot replace on every platform
    if (rename(tmp, path) != 0) { remove(tmp); return false; }
    return true;
}

}  // namespace sok
