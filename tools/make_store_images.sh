#!/usr/bin/env bash
# Regenerates SD_files/SOKOBAN/Picture.png (title screen) and screen.bmp (a game screen) by driving the
# real PC build with a script (headless). Usage: tools/make_store_images.sh path/to/sokoban
set -euo pipefail
BIN="${1:?usage: make_store_images.sh path/to/sokoban}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
python3 - "$TMP/shots.script" <<'PY'
import sys
l = ["10 shot title.bmp", "12 tap a"]    # title screenshot, then "Jouer"
f = 30                                   # pack list: go down to Microcosmos
for _ in range(12): l.append("%d tap down" % f); f += 6
l.append("%d tap a" % f); f += 20    # open the pack
l.append("%d tap a" % f); f += 30    # start level 1
for d in ("left", "left", "left", "left"): l.append("%d tap %s" % (f, d)); f += 14
l.append("%d shot game.bmp" % f); f += 2
l.append("%d quit" % f)
open(sys.argv[1], "w").write("\n".join(l) + "\n")
PY
export SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy SOKOBAN_DATA="$ROOT/SD_files/SOKOBAN" SOKOBAN_SAVE="$TMP/save"
(cd "$TMP" && AKA_PC_SCRIPT="$TMP/shots.script" "$BIN" >/dev/null)
python3 - "$TMP" "$ROOT/SD_files/SOKOBAN" <<'PY'
import sys
from PIL import Image
tmp, out = sys.argv[1:3]
Image.open(tmp + "/title.bmp").convert("RGB").save(out + "/Picture.png", optimize=True)
Image.open(tmp + "/game.bmp").convert("RGB").save(out + "/screen.bmp")
print("wrote Picture.png and screen.bmp")
PY
