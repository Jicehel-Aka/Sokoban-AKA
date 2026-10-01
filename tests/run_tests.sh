#!/usr/bin/env bash
# Runs every test that needs no hardware:
#   1. the engine tests (rules, parser, history, packs, solver) on the real level packs;
#   2. a scripted run of the real SDL build (headless) that plays a level through the UI.
# Usage: tests/run_tests.sh [path-to-sokoban-pc-binary]
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

echo "== engine tests"
g++ -std=c++17 -O2 -Wall -Wextra -I"$ROOT/main/engine" -o "$TMP/engine_test" \
    "$ROOT"/tests/engine_test.cpp "$ROOT"/main/engine/sok_*.cpp
"$TMP/engine_test" "$ROOT/SD_files/SOKOBAN/levelpacks" 3 | tail -n 3

echo "== level editor tests"
g++ -std=c++17 -O2 -Wall -Wextra -I"$ROOT/main" -I"$ROOT/main/engine" -o "$TMP/edit_test" \
    "$ROOT"/tests/edit_test.cpp "$ROOT"/main/engine/sok_*.cpp
"$TMP/edit_test" "$TMP"

BIN="${1:-}"
if [ -n "$BIN" ]; then
  # The runs below happen in a temporary directory (cd): a relative path would no longer resolve.
  BIN="$(cd "$(dirname "$BIN")" && pwd)/$(basename "$BIN")"
  [ -f "$BIN" ] || [ -f "$BIN.exe" ] || { echo "binary not found: $BIN" >&2; exit 1; }
  echo "== scripted UI run"
  export SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy
  export SOKOBAN_DATA="$ROOT/SD_files/SOKOBAN" SOKOBAN_SAVE="$TMP/save"
  (cd "$TMP" && AKA_PC_SCRIPT="$ROOT/tests/pc_play.script" timeout 60 "$BIN")
  test -f "$TMP/solved.bmp" && test -f "$TMP/level2.bmp"
  python3 - "$TMP/save/PROGRESS.DAT" <<'PY'
import struct, sys
d = open(sys.argv[1], "rb").read()
magic, count = struct.unpack_from("<II", d, 0)
assert magic == 0x534B5031 and count >= 1, "bad progress file"
name = d[8:48].split(b"\0")[0].decode()
unlocked = struct.unpack_from("<H", d, 48)[0]
print("progress:", name, "unlocked", unlocked)
assert name == "Initiation" and unlocked == 2
PY
  echo "scripted UI run OK"

  echo "== scripted level editor run"
  mkdir -p "$TMP/edata" "$TMP/eshots"
  cp -r "$ROOT/SD_files/SOKOBAN/." "$TMP/edata/"          # the editor writes into mylevels/: work on a copy
  python3 "$ROOT/tests/make_editor_script.py" "$TMP/editor.script"
  (cd "$TMP/eshots" && SOKOBAN_DATA="$TMP/edata" SOKOBAN_SAVE="$TMP/esave" AKA_PC_SCRIPT="$TMP/editor.script" timeout 60 "$BIN")
  test -f "$TMP/eshots/ed_saved.bmp"
  python3 - "$TMP/edata/mylevels/MYPACK1.sok" <<'PY'
import sys
t = open(sys.argv[1]).read()
print(t)
assert t.startswith("Set: MyPack1\n"), t
assert "#####\n#@$.#\n#####\nTitle: Level 1" in t, "level not saved as drawn"
PY
  echo "scripted editor run OK"
fi
echo "ALL TESTS PASSED"
