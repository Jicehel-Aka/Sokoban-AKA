/*
 * i18n.h - texts in five languages (FR, EN, DE, ES, IT). UTF-8, lowercase accents only
 * (the 8x8 font has no uppercase accented glyph).
 * SPDX-License-Identifier: MIT
 */
#pragma once

namespace i18n {

enum Lang { FR, EN, DE, ES, IT, LANG_COUNT };

enum Str {
    S_PLAY, S_OPTIONS, S_CREDITS, S_QUIT,
    S_CHOOSE_PACK, S_CHOOSE_LEVEL, S_LEVELS_FMT, S_LEVEL_FMT, S_MOVES, S_PUSHES, S_LOCKED,
    S_SOLVED, S_SOLVED_NEXT, S_PACK_DONE,
    S_PAUSE, S_RESUME, S_RESTART, S_LEVEL_SELECT, S_TO_TITLE,
    S_OPT_LANGUAGE, S_OPT_MUSIC, S_OPT_SFX, S_OPT_ANIM, S_OPT_UNLOCK, S_ON, S_OFF, S_BACK,
    S_NO_PACKS, S_LOAD_ERROR, S_SHOT_SAVED, S_SHOT_FAILED, S_BUILTIN_HINT,
    S_HELP_GAME, S_HELP_MENU, S_HELP_LEVELS, S_QUIT_CONFIRM, S_YES, S_NO,
    S_CR_TITLE_GAME, S_CR_TITLE_ART, S_CR_TITLE_SOUND, S_CR_TITLE_LEVELS, S_CR_TITLE_PORT, S_PAGE_FMT,
    // level editor
    S_EDITOR, S_ED_PACKS_TITLE, S_ED_NEW_PACK, S_ED_NEW_LEVEL, S_ED_TEST, S_ED_SAVE, S_ED_DISCARD, S_ED_CLEAR,
    S_ED_SAVED, S_ED_SAVE_FAILED, S_ED_DELETE_CONFIRM, S_ED_TEST_OK,
    S_ED_ERASE, S_ED_WALL, S_ED_GOAL, S_ED_BOX, S_ED_PLAYER, S_ED_HELP, S_ED_LIST_HELP, S_ED_PACK_TOO_BIG,
    S_ST_EMPTY, S_ST_TOOBIG, S_ST_NOPLAYER, S_ST_MANYPLAYERS, S_ST_NOBOX, S_ST_NOGOAL, S_ST_MISMATCH,
    S_COUNT
};

void set_lang(int lang);
int  lang();
const char* lang_name(int lang);          // native name
const char* tr(Str s);

}  // namespace i18n
