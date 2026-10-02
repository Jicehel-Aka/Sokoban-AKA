# Credits & licences

**Sokoban for Gamebuino AKA** is a port of **Sokoban (GP2X)**, written by **Willems Davy
(joyrider3774)**. This file lists everything that comes from somebody else, under which licence,
and what was changed. If you spot a mistake or an omission, please open an issue.

## The original game

- **Title:** Sokoban (GP2X) — "a remake of the classic Sokoban game"
- **Author / copyright:** © 2006–2026 Willems Davy (joyrider3774)
- **Source:** https://github.com/joyrider3774/Sokoban
- **Licence:** MIT (see `LICENSE`, which keeps the original copyright notice)

The rules, the user interface idea (level packs, unlocking one level after the other, undo of up to
1000 moves, several music tracks) and the `.sok` level pack support come from the original.
The code of this port is a **new implementation** in C++ for the Gamebuino AKA library (no source file
of the original is copied); it follows the original behaviour:

- push-only, one box at a time, level solved when every goal holds a box;
- 26 × 15 tiles at most (bigger levels are skipped), 1000 undo steps;
- after a level is solved the next one is unlocked; progress is saved;
- player sprite animation and the same sprites.

### What was changed or left out

- Added: Gamebuino AKA version (ESP32-S3), SDL2 desktop version that runs the *same* game code,
  five languages (FR, EN, DE, ES, IT), smooth-move and "all levels open" options, redo,
  pause menu, screenshot, SD-card layout and release scripts.
- Sprites were **resized** (24, 16 and 12 pixel versions) to fit a 320×240 screen with levels of up to
  26×15 tiles. Music and sounds were **converted** to mono 16-bit 44.1 kHz WAV, the only format the
  AKA audio library plays. No other change was made to them.
- Level editor: re-implemented (`main/engine/sok_edit.*`, screens in `main/app.cpp`). It saves packs as
  standard `.sok` text files in `SOKOBAN/mylevels/`, not in the original game's binary `.lev` format.
- Not included (version 1.0): USB joystick set-up, custom level-pack skins
  and colours, and the 14-level `pimpernel` pack (stored in the original binary `.lev` format, its
  homepage is http://www.pimpernel.com/sokoban/game.html; no redistribution licence is stated in its README,
  and none was found in the repository).
- The original title picture and background pictures are **not** used; the title screen here is
  drawn from the game sprites.

## Graphics

| Asset | Author | Licence |
|-------|--------|---------|
| Wall | [1001.com](https://opengameart.org/content/sokoban-pack) | [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/) |
| Floor, player | [Kenney – Sokoban 100 tiles](https://opengameart.org/content/sokoban-100-tiles) | [CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/) |
| Box | [SpriteAttack – boxes and crates](https://opengameart.org/content/boxes-and-crates-svg-and-pngs) | [CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/) |

The originals are in `assets/original/graphics/`; `tools/gen_tiles.py` turns them into
`main/assets/tiles_data.cpp`. **The wall tile is a CC BY-SA 3.0 work and the resized wall in
`tiles_data.cpp` is an adaptation of it: that adaptation is shared under the same licence
(CC BY-SA 3.0).** The rest of the code is not affected.

## Music (not in the git repository as WAV: converted at release time from `assets/music/*.ogg`)

| File on the SD card | Original title | Author | Licence | Source |
|---|---|---|---|---|
| `title.wav` | "title" | migfus20 | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) | https://opengameart.org/content/weird-shop-gypsy-guitar |
| `puzzle3.wav` | "Puzzle Game 3" | Eric Matyas (soundimage.org) | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) | https://opengameart.org/content/puzzle-game-3 |
| `periwink.wav` | "periwinkle" | axtoncrolley | [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/) | https://opengameart.org/content/happy-go-lucky-puzzle |
| `calmbgm.wav` | "041415calmbgm" | syncopika | [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/) | https://opengameart.org/content/calm-bgm |

Changes: decoded from Ogg Vorbis, mixed down to mono, written as WAV (44.1 kHz, 16 bit).

## Sound effects (`SD_files/SOKOBAN/sound/`)

| File | Author | Licence | Source |
|---|---|---|---|
| `stageend.wav` | Fupi | [CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/) | https://opengameart.org/content/win-jingle |
| `select.wav`, `back.wav` | ViRiX Dreamcore (David McKee), soundcloud.com/virix | [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/) | https://opengameart.org/content/ui-and-item-sounds-sample-1 |
| `error.wav` | ViRiX Dreamcore (David McKee) | [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/) | https://opengameart.org/content/ui-failed-or-error |
| `menu.wav` | Tim Mortimer | [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/) | https://opengameart.org/content/4-sci-fi-menu-sounds |
| `move.wav` | Willems Davy (made with BXFR) | "feel free to use" (original author's words) | the original repository |

Changes: mixed down to mono where they were stereo. Originals in `assets/original/sound/`; the
original credit files are `assets/original/music_credits.txt` and `sound_credits.txt`.

## Level packs (`SD_files/SOKOBAN/levelpacks/*.sok`)

The packs are **unmodified** `.sok` files as distributed by https://www.sokobano.de/ and bundled
with the original game. Each file keeps its own header (set name, author, e-mail, homepage). Most of
them state: *"This Sokoban level set is published here with the author's permission."*

**The levels remain © their authors.** They are included because the original project distributes
them with attribution; the permission quoted above was given to the Sokoban site, so if you
redistribute this package yourself, check with the authors (links below) that this suits them, or
remove the packs you are unsure about: the game works with any set of `.sok` files in
`SOKOBAN/levelpacks/`.

| Pack(s) | Author | Homepage |
|---|---|---|
| Minicosmos, Microcosmos, Nabokosmos, Picokosmos, Cosmopoly, Myriocosmos, Cosmonotes | Aymeric du Peloux | https://aymericdupeloux.wixsite.com/sokoban |
| GRIGoRusha 2001, 2002, Remodel Club, Special, Star, Sun | Evgeniy Grigoriev (GRIGoRusha) | http://grigr.narod.ru/ |
| SokEvo, SokHard, SokWhole | Lee J Haywood | https://ljhaywood.uk/games/sokoban/ |
| LOMA | Aymeric du Peloux (set header) — listed under Lee J Haywood's page in the original README | http://membres.lycos.fr/nabokos |
| Erim Sever Collection | Erim Sever | https://web.archive.org/web/20191029215423/http://www.erimsever.com/e_sokoban.htm |
| 696 | Dries de Clercq | – |
| *Initiation* (6 levels, built in) | written for this port (MIT) | – |

## Gamebuino AKA library — `components/gamebuino/`

- © Gamebuino 2026, author Jean-Marie Papillon — **GNU LGPL v3 or later** (texts in `THIRD_PARTY/`).
- Used here unchanged, except for the additions made earlier by the AKA project (the `drawImage`
  family in `include_lib/gb_graphics_image.cpp`, the case-fix script `tools/fix_gamebuino_case.py`).
- The PC build compiles this very library on the desktop and replaces only the hardware layer
  `gb_ll_*` by `pc/gb_ll_pc.cpp` (SDL2).
- Font: `font8x8_basic` by Daniel Hepper, based on public-domain VGA fonts — public domain. The
  accented glyphs (`main/assets/font_accents.*`, generated by `tools/gen_font.py`) are part of the AKA
  game collection.

## Desktop version

- **SDL2** — zlib licence (`THIRD_PARTY/SDL2-zlib.txt`); Linux uses the system library, the Windows
  package ships `SDL2.dll`.

## Port

- Gamebuino AKA / SDL2 port: **Jicehel**, 2026, with the help of Claude (Anthropic) for the code.
