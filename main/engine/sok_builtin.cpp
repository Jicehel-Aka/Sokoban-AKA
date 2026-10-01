/*
 * sok_builtin.cpp - original introductory levels written for Sokoban-AKA
 * (not taken from any existing collection). Each one is checked solvable by
 * tests/engine_test.cpp.
 * SPDX-License-Identifier: MIT
 */
#include "sok_builtin.h"

namespace sok {

const char* const BUILTIN_PACK_NAME = "Initiation";

const char* const BUILTIN_PACK_TEXT =
    "Set: Initiation\n"
    "Author: Sokoban-AKA\n"
    "\n"
    "#####\n"
    "#@$.#\n"
    "#####\n"
    "Title: First push\n"
    "\n"
    "#######\n"
    "#     #\n"
    "# .$@ #\n"
    "#     #\n"
    "#######\n"
    "Title: Push, never pull\n"
    "\n"
    "  #####\n"
    "###   #\n"
    "# $ # #\n"
    "# .@  #\n"
    "###  ##\n"
    "  ####\n"
    "Title: Around the corner\n"
    "\n"
    "#######\n"
    "#     #\n"
    "# $ $ #\n"
    "# . . #\n"
    "#  @  #\n"
    "#######\n"
    "Title: Two boxes\n"
    "\n"
    "########\n"
    "#      #\n"
    "#  $$  #\n"
    "# .@.  #\n"
    "#      #\n"
    "########\n"
    "Title: Side by side\n"
    "\n"
    "#######\n"
    "#  .  #\n"
    "#  $  #\n"
    "#.$@$ #\n"
    "#  .  #\n"
    "#######\n"
    "Title: Three crates\n";

}  // namespace sok
