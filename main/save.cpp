/*
 * save.cpp - see save.h. Files are small binary records with a magic number; a damaged or
 * missing file just means "defaults".
 * SPDX-License-Identifier: MIT
 */
#include "save.h"

#include <stdio.h>
#include <string.h>

#include "platform.h"

namespace save {

static const uint32_t CFG_MAGIC = 0x534B4F31u;     // "SKO1"
static const uint32_t PRG_MAGIC = 0x534B5031u;     // "SKP1"
static const int MAX_PACKS = 64;

struct CfgFile { uint32_t magic; Config cfg; };
struct Entry { char name[40]; uint16_t unlocked; uint16_t pad; };
struct PrgFile { uint32_t magic; uint32_t count; Entry e[MAX_PACKS]; };

static PrgFile s_prg;
static bool s_prg_loaded = false;

bool load_config(Config& c)
{
    char path[600];
    plat::save_path(path, sizeof path, "CFG.DAT");
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    CfgFile d {};
    const bool ok = fread(&d, sizeof d, 1, f) == 1;
    fclose(f);
    if (!ok || d.magic != CFG_MAGIC) return false;
    d.cfg.last_pack[sizeof d.cfg.last_pack - 1] = 0;
    if (d.cfg.music > 10) d.cfg.music = 10;
    if (d.cfg.sfx > 10) d.cfg.sfx = 10;
    c = d.cfg;
    return true;
}

bool save_config(const Config& c)
{
    plat::make_dir(plat::save_dir());
    char path[600];
    plat::save_path(path, sizeof path, "CFG.DAT");
    FILE* f = fopen(path, "wb");
    if (!f) return false;
    CfgFile d {};
    d.magic = CFG_MAGIC;
    d.cfg = c;
    const bool ok = fwrite(&d, sizeof d, 1, f) == 1;
    fclose(f);
    return ok;
}

static void load_progress()
{
    if (s_prg_loaded) return;
    s_prg_loaded = true;
    memset(&s_prg, 0, sizeof s_prg);
    char path[600];
    plat::save_path(path, sizeof path, "PROGRESS.DAT");
    FILE* f = fopen(path, "rb");
    if (!f) return;
    PrgFile d;
    const bool ok = fread(&d, sizeof d, 1, f) == 1;
    fclose(f);
    if (ok && d.magic == PRG_MAGIC && d.count <= MAX_PACKS) s_prg = d;
}

static int find(const char* pack)
{
    for (uint32_t i = 0; i < s_prg.count; ++i)
        if (strncmp(s_prg.e[i].name, pack, sizeof s_prg.e[i].name - 1) == 0) return (int)i;
    return -1;
}

int unlocked(const char* pack, int level_count)
{
    load_progress();
    const int i = find(pack);
    int n = i < 0 ? 1 : s_prg.e[i].unlocked;
    if (n > level_count) n = level_count;
    return n < 1 ? 1 : n;
}

void set_unlocked(const char* pack, int count)
{
    load_progress();
    int i = find(pack);
    if (i < 0) {
        if (s_prg.count >= MAX_PACKS) return;
        i = (int)s_prg.count++;
        memset(&s_prg.e[i], 0, sizeof s_prg.e[i]);
        strncpy(s_prg.e[i].name, pack, sizeof s_prg.e[i].name - 1);
    }
    if (count <= s_prg.e[i].unlocked) return;
    s_prg.e[i].unlocked = (uint16_t)count;
    s_prg.magic = PRG_MAGIC;
    plat::make_dir(plat::save_dir());
    char path[600];
    plat::save_path(path, sizeof path, "PROGRESS.DAT");
    FILE* f = fopen(path, "wb");
    if (!f) return;
    fwrite(&s_prg, sizeof s_prg, 1, f);
    fclose(f);
}

}  // namespace save
