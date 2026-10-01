/*
 * sok_builtin.h - a tiny level pack compiled into the firmware, so the game is
 * playable even when the SD card has no levelpacks folder.
 * SPDX-License-Identifier: MIT
 */
#pragma once

namespace sok {
// Text of the built-in pack (.sok format).
extern const char* const BUILTIN_PACK_TEXT;
extern const char* const BUILTIN_PACK_NAME;
}  // namespace sok
