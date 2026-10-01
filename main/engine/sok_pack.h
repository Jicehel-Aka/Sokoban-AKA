/*
 * sok_pack.h - level packs in the standard ".sok" text format (sokobano.de).
 *
 * Part of Sokoban-AKA, a port of "Sokoban (GP2X)" by Willems Davy (joyrider3774)
 * (MIT). The original game reads the same .sok files; this is a new parser.
 *
 * A pack is loaded into memory once; the level index only stores offsets, so a
 * 700-level pack costs a few kilobytes on top of the file itself.
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <vector>

#include "sok_board.h"

namespace sok {

class Pack {
public:
    Pack() = default;
    ~Pack() { clear(); }
    Pack(const Pack&) = delete;
    Pack& operator=(const Pack&) = delete;

    // Reads a file (any size up to max_bytes) and indexes its levels.
    bool load_file(const char* path, size_t max_bytes = 512 * 1024);
    // Same, from memory (the text is copied). Latin-1 text is converted to UTF-8.
    bool parse_text(const char* text, size_t len);
    void clear();

    int count() const { return (int)levels_.size(); }
    // Levels that were skipped because they are too big or invalid.
    int skipped() const { return skipped_; }

    const char* set_name() const { return set_; }    // "Set:" header, may be empty
    const char* author() const { return author_; }   // pack author, may be empty

    bool get_board(int index, Board& out) const;
    // Metadata of a level; the strings are empty when absent. `out` is always NUL-terminated.
    void get_title(int index, char* out, size_t n) const;
    void get_author(int index, char* out, size_t n) const;   // falls back to the pack author
    void get_comment(int index, char* out, size_t n) const;

private:
    struct LevelRef {
        uint32_t board_off;
        uint32_t meta_off;
        uint16_t board_len;
        uint16_t meta_len;
    };
    bool field(int index, const char* key, char* out, size_t n) const;

    char* text_ = nullptr;
    size_t len_ = 0;
    std::vector<LevelRef> levels_;
    int skipped_ = 0;
    char set_[64] = "";
    char author_[64] = "";
};

// Converts Latin-1 (ISO-8859-1) to UTF-8. Returns the new length; `dst` needs 2*len+1 bytes.
size_t latin1_to_utf8(const char* src, size_t len, char* dst);
// True if `s` is well-formed UTF-8.
bool is_valid_utf8(const char* s, size_t len);

}  // namespace sok
