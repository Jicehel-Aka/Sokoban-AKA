/*
 * sok_pack.cpp - see sok_pack.h.
 * SPDX-License-Identifier: MIT
 */
#include "sok_pack.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace sok {

bool is_valid_utf8(const char* s, size_t len)
{
    size_t i = 0;
    while (i < len) {
        const uint8_t c = (uint8_t)s[i];
        int extra;
        if (c < 0x80) { ++i; continue; }
        else if ((c & 0xE0) == 0xC0 && c >= 0xC2) extra = 1;
        else if ((c & 0xF0) == 0xE0) extra = 2;
        else if ((c & 0xF8) == 0xF0 && c <= 0xF4) extra = 3;
        else return false;
        if (i + (size_t)extra >= len) return false;     // truncated sequence
        for (int k = 1; k <= extra; ++k)
            if (((uint8_t)s[i + k] & 0xC0) != 0x80) return false;
        i += extra + 1;
    }
    return true;
}

size_t latin1_to_utf8(const char* src, size_t len, char* dst)
{
    size_t o = 0;
    for (size_t i = 0; i < len; ++i) {
        const uint8_t c = (uint8_t)src[i];
        if (c < 0x80) dst[o++] = (char)c;
        else { dst[o++] = (char)(0xC0 | (c >> 6)); dst[o++] = (char)(0x80 | (c & 0x3F)); }
    }
    dst[o] = '\0';
    return o;
}

void Pack::clear()
{
    free(text_);
    text_ = nullptr;
    len_ = 0;
    levels_.clear();
    levels_.shrink_to_fit();
    skipped_ = 0;
    set_[0] = author_[0] = '\0';
}

bool Pack::load_file(const char* path, size_t max_bytes)
{
    clear();
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0 || (size_t)sz > max_bytes) { fclose(f); return false; }
    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return false; }
    const size_t got = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[got] = '\0';
    const bool ok = parse_text(buf, got);
    free(buf);
    return ok;
}

namespace {

struct Line { const char* p; size_t n; };

// Next line of [p, end): returns false at the end. Strips the terminating '\n' / '\r'.
bool next_line(const char*& p, const char* end, Line& l)
{
    if (p >= end) return false;
    const char* s = p;
    while (p < end && *p != '\n') ++p;
    size_t n = (size_t)(p - s);
    if (p < end) ++p;                       // skip '\n'
    while (n > 0 && s[n - 1] == '\r') --n;
    l.p = s; l.n = n;
    return true;
}

// A board row only holds board characters and at least one wall; this keeps
// the free text of pack headers ("Enjoy!", "; 1 L", ...) out of the levels.
bool is_board_row(const Line& l)
{
    if (l.n == 0) return false;
    bool wall = false;
    for (size_t i = 0; i < l.n; ++i) {
        if (!is_board_char(l.p[i])) return false;
        if (l.p[i] == '#') wall = true;
    }
    return wall;
}

bool starts_with_ci(const Line& l, const char* key)
{
    const size_t k = strlen(key);
    if (l.n < k) return false;
    for (size_t i = 0; i < k; ++i)
        if (tolower((unsigned char)l.p[i]) != tolower((unsigned char)key[i])) return false;
    return true;
}

void copy_trim(const char* s, size_t n, char* out, size_t cap)
{
    if (cap == 0) return;
    while (n > 0 && (*s == ' ' || *s == '\t')) { ++s; --n; }
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\r')) --n;
    if (n >= cap) {                          // truncate without cutting a UTF-8 sequence
        n = cap - 1;
        while (n > 0 && ((uint8_t)s[n] & 0xC0) == 0x80) --n;
    }
    memcpy(out, s, n);
    out[n] = '\0';
}

}  // namespace

bool Pack::parse_text(const char* text, size_t len)
{
    clear();
    if (!text || len == 0) return false;

    // Level sets come in both Latin-1 and UTF-8; the UI font works on UTF-8.
    if (is_valid_utf8(text, len)) {
        text_ = (char*)malloc(len + 1);
        if (!text_) return false;
        memcpy(text_, text, len);
        text_[len] = '\0';
        len_ = len;
    } else {
        text_ = (char*)malloc(len * 2 + 1);
        if (!text_) return false;
        len_ = latin1_to_utf8(text, len, text_);
    }

    const char* const base = text_;
    const char* const end = text_ + len_;
    const char* p = base;
    Line l;

    // Pass 1: header ("Set:", "Author:") and the board rows of every level.
    bool header_done = false;
    while (true) {
        const char* line_start = p;
        if (!next_line(p, end, l)) break;

        if (!is_board_row(l)) {
            if (!header_done && levels_.empty()) {
                if (set_[0] == '\0' && starts_with_ci(l, "set:"))
                    copy_trim(l.p + 4, l.n - 4, set_, sizeof(set_));
                else if (author_[0] == '\0' && starts_with_ci(l, "author:"))
                    copy_trim(l.p + 7, l.n - 7, author_, sizeof(author_));
            }
            continue;
        }

        // A run of board rows is one level.
        const char* board_start = line_start;
        const char* board_end = l.p + l.n;
        int rows = 1;
        while (true) {
            const char* save = p;
            Line m;
            if (!next_line(p, end, m)) break;
            if (!is_board_row(m)) { p = save; break; }
            board_end = m.p + m.n;
            ++rows;
        }
        header_done = true;

        // Metadata lines run until the next level starts.
        const char* meta_start = p;
        const char* q = p;
        const char* meta_end = p;
        while (true) {
            const char* ls = q;
            Line m;
            if (!next_line(q, end, m)) break;
            if (is_board_row(m)) { q = ls; break; }
            meta_end = m.p + m.n;
        }

        Board b;
        const ParseStatus st = parse_board(board_start, (size_t)(board_end - board_start), b);
        if (rows >= 3 && st == ParseStatus::Ok && (board_end - board_start) < 65535 &&
            (meta_end - meta_start) >= 0 && (meta_end - meta_start) < 65535) {
            LevelRef r;
            r.board_off = (uint32_t)(board_start - base);
            r.board_len = (uint16_t)(board_end - board_start);
            r.meta_off = (uint32_t)(meta_start - base);
            r.meta_len = (uint16_t)(meta_end > meta_start ? meta_end - meta_start : 0);
            levels_.push_back(r);
        } else {
            ++skipped_;
        }
        p = q;
    }

    // Pack author: header value, else the first level's author.
    if (author_[0] == '\0' && !levels_.empty()) {
        char tmp[64];
        if (field(0, "author", tmp, sizeof(tmp))) strcpy(author_, tmp);
    }
    return !levels_.empty();
}

bool Pack::get_board(int index, Board& out) const
{
    if (index < 0 || index >= count()) return false;
    const LevelRef& r = levels_[(size_t)index];
    return parse_board(text_ + r.board_off, r.board_len, out) == ParseStatus::Ok;
}

// Looks for "key:" in the metadata of a level. "Comment:" runs until "Comment-End:".
bool Pack::field(int index, const char* key, char* out, size_t n) const
{
    if (n) out[0] = '\0';
    if (index < 0 || index >= count()) return false;
    const LevelRef& r = levels_[(size_t)index];
    const char* p = text_ + r.meta_off;
    const char* end = p + r.meta_len;
    const size_t klen = strlen(key);
    Line l;
    while (next_line(p, end, l)) {
        if (!starts_with_ci(l, key) || l.n <= klen || l.p[klen] != ':') continue;
        // value on the same line
        char tmp[256];
        copy_trim(l.p + klen + 1, l.n - klen - 1, tmp, sizeof(tmp));
        if (strcmp(key, "comment") == 0) {
            // append the following lines until "comment-end:"
            size_t used = strlen(tmp);
            Line m;
            while (next_line(p, end, m)) {
                if (starts_with_ci(m, "comment-end")) break;
                if (used + 2 < sizeof(tmp)) {
                    if (used) tmp[used++] = ' ';
                    char part[128];
                    copy_trim(m.p, m.n, part, sizeof(part));
                    size_t pl = strlen(part);
                    if (used + pl >= sizeof(tmp)) pl = sizeof(tmp) - 1 - used;
                    memcpy(tmp + used, part, pl);
                    used += pl;
                    tmp[used] = '\0';
                }
            }
        }
        copy_trim(tmp, strlen(tmp), out, n);
        return out[0] != '\0';
    }
    return false;
}

void Pack::get_title(int index, char* out, size_t n) const { field(index, "title", out, n); }
void Pack::get_comment(int index, char* out, size_t n) const { field(index, "comment", out, n); }

void Pack::get_author(int index, char* out, size_t n) const
{
    if (!field(index, "author", out, n)) copy_trim(author_, strlen(author_), out, n);
}

}  // namespace sok
