// Tests of the level editor model: placing parts, conversion, validation, and a write/read round trip
// through the real pack loader.  SPDX-License-Identifier: MIT
#include <stdio.h>
#include <string.h>

#include "engine/sok_edit.h"
#include "engine/sok_game.h"
#include "engine/sok_pack.h"

static int g_checks = 0, g_fail = 0;
#define CHECK(c) do { ++g_checks; if (!(c)) { ++g_fail; printf("FAIL line %d: %s\n", __LINE__, #c); } } while (0)

using namespace sok;

static void paint(Grid& g, int y, const char* row)
{
    for (int x = 0; row[x]; ++x) {
        switch (row[x]) {
            case '#': g.place(x, y, Part::Wall); break;
            case '$': g.place(x, y, Part::Box); break;
            case '.': g.place(x, y, Part::Goal); break;
            case '@': g.place(x, y, Part::Player); break;
            case '*': g.place(x, y, Part::Box); g.place(x, y, Part::Goal); break;
            default: break;
        }
    }
}

int main(int argc, char** argv)
{
    Grid g;
    CHECK(g.empty());
    char t[512];
    CHECK(g.to_text(t, sizeof t) == 0);

    // combine / replace rules
    g.place(2, 2, Part::Box);   CHECK(g.c[2][2] == '$');
    g.place(2, 2, Part::Goal);  CHECK(g.c[2][2] == '*');
    g.place(2, 2, Part::Player); CHECK(g.c[2][2] == '+');            // box replaced by player, goal kept
    g.place(5, 5, Part::Player); CHECK(g.c[5][5] == '@' && g.c[2][2] == '.');   // only one player
    g.place(5, 5, Part::Box);   CHECK(g.c[5][5] == '$');
    g.place(5, 5, Part::Wall);  CHECK(g.c[5][5] == '#');
    g.place(5, 5, Part::Goal);  CHECK(g.c[5][5] == '.');             // goal on a wall: the wall goes away
    g.place(5, 5, Part::Floor); CHECK(g.c[5][5] == ' ');
    g.place(-1, 0, Part::Wall); g.place(MAX_W, 0, Part::Wall); g.place(0, MAX_H, Part::Wall);   // ignored
    g.place(0, 0, Part::Wall);  CHECK(g.c[0][0] == '#');

    // a valid level
    Grid a;
    paint(a, 3, "  #####");
    paint(a, 4, "  #@$.#");
    paint(a, 5, "  #####");
    Board b;
    CHECK(a.validate(&b) == ParseStatus::Ok);
    CHECK(b.w == 5 && b.h == 3 && b.boxes == 1 && b.goals == 1 && b.px == 1 && b.py == 1);
    CHECK(a.to_text(t, sizeof t) == 17);                              // 3 rows of 5 + 2 newlines
    CHECK(strcmp(t, "#####\n#@$.#\n#####") == 0);

    // validation errors
    Grid e = a; e.place(4, 4, Part::Floor);                           // box removed
    CHECK(e.validate() == ParseStatus::NoBox);
    e = a; e.place(5, 4, Part::Floor);                                // goal removed
    CHECK(e.validate() == ParseStatus::NoGoal);
    e = a; e.place(3, 4, Part::Floor);                                // player removed
    CHECK(e.validate() == ParseStatus::NoPlayer);
    e = a; e.place(6, 4, Part::Box); e.place(6, 4, Part::Goal);       // a second box already on a goal
    CHECK(e.validate() == ParseStatus::Ok);
    e = a; e.place(6, 4, Part::Goal);                                 // more goals than boxes
    CHECK(e.validate() == ParseStatus::BoxGoalMismatch);
    Grid w; w.place(1, 1, Part::Box);
    CHECK(w.validate() != ParseStatus::Ok);                           // no wall

    // board -> grid -> board is the identity
    Grid back;
    CHECK(back.from_board(b));
    Board b2;
    CHECK(back.validate(&b2) == ParseStatus::Ok);
    CHECK(b2.w == b.w && b2.h == b.h && b2.px == b.px && b2.py == b.py);
    for (int y = 0; y < b.h; ++y) for (int x = 0; x < b.w; ++x) CHECK((b.cell[y][x] & 7) == (b2.cell[y][x] & 7));

    // level in the biggest allowed area
    Grid big;
    for (int x = 0; x < MAX_W; ++x) { big.place(x, 0, Part::Wall); big.place(x, MAX_H - 1, Part::Wall); }
    for (int y = 0; y < MAX_H; ++y) { big.place(0, y, Part::Wall); big.place(MAX_W - 1, y, Part::Wall); }
    big.place(1, 1, Part::Player); big.place(2, 1, Part::Box); big.place(3, 1, Part::Goal);
    CHECK(big.validate(&b) == ParseStatus::Ok && b.w == MAX_W && b.h == MAX_H);

    // write -> Pack -> play: the editor output is a normal pack
    static Grid levels[3];
    levels[0] = a;
    levels[1].clear();                                                // empty: skipped by the writer
    levels[2] = big;
    char path[256];
    snprintf(path, sizeof path, "%s/edit_test.sok", argc > 1 ? argv[1] : ".");
    CHECK(write_sok(path, "My test", "Me", levels, 3));
    Pack p;
    CHECK(p.load_file(path));
    CHECK(p.count() == 2);
    CHECK(strcmp(p.set_name(), "My test") == 0 && strcmp(p.author(), "Me") == 0);
    char title[64];
    p.get_title(1, title, sizeof title);
    CHECK(strcmp(title, "Level 3") == 0);
    Board pb;
    CHECK(p.get_board(0, pb));
    Game game;
    game.start(pb);
    CHECK(game.move(DIR_RIGHT) && game.solved());                     // push the box onto the goal
    // overwriting keeps working and leaves no temp file
    CHECK(write_sok(path, "My test", "Me", levels, 1));
    Pack p2;
    CHECK(p2.load_file(path) && p2.count() == 1);
    char tmp[300];
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    FILE* f = fopen(tmp, "rb");
    CHECK(f == nullptr);
    if (f) fclose(f);
    CHECK(!write_sok("/nonexistent-dir/x.sok", "a", "b", levels, 1));
    remove(path);

    printf("editor: %d checks, %d failed\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
