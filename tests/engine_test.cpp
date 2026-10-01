// engine_test.cpp - unit tests for the hardware independent Sokoban engine.
// Build & run: see tests/run_tests.sh.   SPDX-License-Identifier: MIT
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include "../main/engine/sok_board.h"
#include "../main/engine/sok_builtin.h"
#include "../main/engine/sok_game.h"
#include "../main/engine/sok_pack.h"

using namespace sok;

static int g_checks = 0, g_fail = 0;
#define CHECK(c) do { ++g_checks; if (!(c)) { ++g_fail; fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)

static Board board_of(const char* s)
{
    Board b;
    ParseStatus st = parse_board(s, strlen(s), b);
    if (st != ParseStatus::Ok) { fprintf(stderr, "bad test board: %s\n", parse_status_name(st)); abort(); }
    return b;
}

static bool same_board(const Board& a, const Board& b)
{
    if (a.w != b.w || a.h != b.h || a.px != b.px || a.py != b.py) return false;
    for (int y = 0; y < a.h; ++y)
        for (int x = 0; x < a.w; ++x)
            if (a.cell[y][x] != b.cell[y][x]) return false;
    return true;
}

// ---------------------------------------------------------------- parser
static void test_parse()
{
    Board b;
    CHECK(parse_board("#####\n#@$.#\n#####\n", 18, b) == ParseStatus::Ok);
    CHECK(b.w == 5 && b.h == 3 && b.px == 1 && b.py == 1 && b.boxes == 1 && b.goals == 1);
    CHECK(b.is_box(2, 1) && b.is_goal(3, 1) && b.is_wall(0, 0));
    CHECK(b.cell[1][1] & F_INSIDE);
    CHECK(!(b.cell[0][0] & F_INSIDE));

    // '+' = player on goal, '*' = box on goal, indentation is trimmed
    const char* t = "   #######\n   #+*$$.#\n   #######";
    CHECK(parse_board(t, strlen(t), b) == ParseStatus::Ok);
    CHECK(b.w == 7 && b.px == 1 && b.goals == 3 && b.boxes == 3 && b.boxes_on_goals() == 1);
    CHECK(b.is_goal(1, 1));

    CHECK(parse_board("###\n# #\n###", 11, b) == ParseStatus::NoPlayer);
    CHECK(parse_board("#####\n#@@$.#\n#####", 18, b) == ParseStatus::ManyPlayers);
    CHECK(parse_board("#####\n#@ .#\n#####", 16, b) == ParseStatus::NoBox);
    CHECK(parse_board("#####\n#@$ #\n#####", 16, b) == ParseStatus::NoGoal);
    CHECK(parse_board("######\n#@$$.#\n######", 20, b) == ParseStatus::Ok);   // more boxes than goals is allowed
    CHECK(parse_board("#######\n#@$..##\n#######", 22, b) == ParseStatus::BoxGoalMismatch);
    CHECK(parse_board("", 0, b) == ParseStatus::Empty);
    CHECK(parse_board("   \n  ", 6, b) == ParseStatus::Empty);

    // size limits: 26 x 15 is accepted, 27 wide / 16 high are not
    std::string wide(26, '#'), wide27(27, '#');
    std::string ok = wide + "\n#@$." + std::string(21, ' ') + "#\n" + wide;
    CHECK(parse_board(ok.data(), ok.size(), b) == ParseStatus::Ok && b.w == 26);
    std::string bad = wide27 + "\n#@$." + std::string(22, ' ') + "#\n" + wide27;
    CHECK(parse_board(bad.data(), bad.size(), b) == ParseStatus::TooBig);
    std::string tall = "#####\n#@$.#\n";
    for (int i = 0; i < 14; ++i) tall += "#   #\n";
    tall += "#####";
    CHECK(parse_board(tall.data(), tall.size(), b) == ParseStatus::TooBig);

    // CRLF line endings
    CHECK(parse_board("#####\r\n#@$.#\r\n#####\r\n", 21, b) == ParseStatus::Ok && b.w == 5);

    // floor outside the walls is not "inside"
    const char* o = "  ###\n###@#\n# $.#\n#####";
    CHECK(parse_board(o, strlen(o), b) == ParseStatus::Ok);
    CHECK(!(b.cell[0][0] & F_INSIDE));
    CHECK(b.cell[2][1] & F_INSIDE);
}

// ---------------------------------------------------------------- rules
static void test_rules()
{
    Game g;
    g.start(board_of("#####\n#@$.#\n#####"));
    Step s;
    CHECK(!g.solved());
    CHECK(!g.move(DIR_UP, &s));                    // wall
    CHECK(g.moves() == 0);
    CHECK(g.move(DIR_RIGHT, &s) && s.pushed && s.dir == DIR_RIGHT);
    CHECK(g.moves() == 1 && g.pushes() == 1);
    CHECK(g.board().is_box(3, 1) && g.board().px == 2);
    CHECK(g.solved());
    CHECK(!g.move(DIR_RIGHT));                     // box against the wall
    CHECK(g.moves() == 1);
    CHECK(g.undo(&s) && s.pushed);
    CHECK(g.moves() == 0 && g.pushes() == 0 && !g.solved());
    CHECK(g.board().is_box(2, 1) && g.board().px == 1);
    CHECK(!g.undo());
    CHECK(g.redo());
    CHECK(g.solved());
    CHECK(!g.redo());
    g.restart();
    CHECK(g.moves() == 0 && g.board().is_box(2, 1) && !g.can_undo() && !g.can_redo());

    // two boxes in a row cannot be pushed; a box cannot enter a wall
    g.start(board_of("#######\n#@$$ .#\n#######"));
    CHECK(!g.move(DIR_RIGHT));
    CHECK(g.moves() == 0 && g.facing() == DIR_RIGHT);

    // already solved at start: not "solved" until the player moves (original behaviour)
    g.start(board_of("#####\n#@ *#\n#####"));
    CHECK(g.all_goals_covered() && !g.solved());
    CHECK(g.move(DIR_RIGHT) && g.solved());

    // a new move discards the redo branch
    g.start(board_of("#######\n#     #\n# @$. #\n#     #\n#######"));
    CHECK(g.move(DIR_LEFT));
    CHECK(g.move(DIR_UP));
    CHECK(g.undo());
    CHECK(g.can_redo());
    CHECK(g.move(DIR_DOWN));
    CHECK(!g.can_redo());
    CHECK(g.moves() == 2);
}

// -------------------------------------------------- randomized undo/redo
static uint32_t rng_state = 12345;
static uint32_t rnd() { rng_state = rng_state * 1664525u + 1013904223u; return rng_state >> 8; }

static void test_fuzz()
{
    const char* lv = "  #####\n###   #\n#  $  #\n# .*@ #\n#  $. #\n# $.  #\n#######";
    Board start = board_of(lv);
    Game g;
    g.start(start);
    std::vector<Board> snaps;
    snaps.push_back(g.board());
    int depth = 0;
    for (int i = 0; i < 200000; ++i) {
        const uint32_t r = rnd() % 10;
        if (r < 6) {
            if (g.move((Dir)(rnd() & 3))) {
                if (depth < MAX_HISTORY) { snaps.push_back(g.board()); ++depth; }
                else { snaps.erase(snaps.begin()); snaps.push_back(g.board()); }
            }
        } else if (r < 9) {
            if (g.undo()) {
                snaps.pop_back();
                --depth;
                CHECK(same_board(g.board(), snaps.back()));
                if (!same_board(g.board(), snaps.back())) return;
                // redo must come back to the same position
                Board before = g.board();
                CHECK(g.redo());
                CHECK(g.undo());
                CHECK(same_board(g.board(), before));
            } else {
                CHECK(depth == 0);
            }
        } else {
            // box count and goal count never change
            CHECK(g.board().boxes_on_goals() <= (int)g.board().goals);
        }
        int boxes = 0;
        for (int y = 0; y < g.board().h; ++y)
            for (int x = 0; x < g.board().w; ++x)
                if (g.board().cell[y][x] & F_BOX) ++boxes;
        if (boxes != (int)start.boxes) { CHECK(boxes == (int)start.boxes); return; }
    }
}

static void test_history_limit()
{
    Game g;
    g.start(board_of("#########\n#@      #\n#  $   .#\n#########"));
    for (int i = 0; i < 1500; ++i) CHECK(g.move(i % 2 ? DIR_LEFT : DIR_RIGHT));
    int undone = 0;
    while (g.undo()) ++undone;
    CHECK(undone == MAX_HISTORY);
    CHECK(g.moves() == 1500 - MAX_HISTORY);
}

// ---------------------------------------------------------------- solver
// Push-based BFS, used only to prove that levels are solvable with *our* rules.
struct SState { std::vector<uint16_t> boxes; uint16_t player; int parent; int via_box_dir; };

static bool solve(const Board& b, std::vector<Dir>& moves, size_t cap = 400000)
{
    const int W = b.w, H = b.h;
    auto idx = [&](int x, int y) { return (uint16_t)(y * W + x); };
    std::vector<uint8_t> wall(W * H), goal(W * H);
    std::vector<uint16_t> start_boxes;
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            wall[idx(x, y)] = (b.cell[y][x] & F_WALL) != 0;
            goal[idx(x, y)] = (b.cell[y][x] & F_GOAL) != 0;
            if (b.cell[y][x] & F_BOX) start_boxes.push_back(idx(x, y));
        }
    auto key_of = [&](const std::vector<uint16_t>& bx, uint16_t pl) {
        std::string k((const char*)bx.data(), bx.size() * 2);
        k.append((const char*)&pl, 2);
        return k;
    };
    static const int dx[4] = {1, 0, -1, 0}, dy[4] = {0, 1, 0, -1};

    auto reach = [&](const std::vector<uint16_t>& bx, uint16_t pl, std::vector<int>& dist, std::vector<int>& from) {
        std::vector<uint8_t> occ(W * H, 0);
        for (auto v : bx) occ[v] = 1;
        dist.assign(W * H, -1);
        from.assign(W * H, -1);
        std::deque<uint16_t> q;
        dist[pl] = 0; q.push_back(pl);
        while (!q.empty()) {
            uint16_t c = q.front(); q.pop_front();
            int x = c % W, y = c / W;
            for (int d = 0; d < 4; ++d) {
                int nx = x + dx[d], ny = y + dy[d];
                if (nx < 0 || ny < 0 || nx >= W || ny >= H) continue;
                uint16_t n = idx(nx, ny);
                if (wall[n] || occ[n] || dist[n] >= 0) continue;
                dist[n] = dist[c] + 1; from[n] = c; q.push_back(n);
            }
        }
        return occ;
    };

    std::vector<SState> states;
    std::unordered_map<std::string, int> seen;
    {
        std::vector<uint16_t> bx = start_boxes;
        std::sort(bx.begin(), bx.end());
        std::vector<int> dist, from;
        reach(bx, idx(b.px, b.py), dist, from);
        uint16_t norm = 0xFFFF;
        for (int i = 0; i < W * H; ++i) if (dist[i] >= 0) { norm = (uint16_t)i; break; }
        states.push_back({bx, norm, -1, 0});
        seen[key_of(bx, norm)] = 0;
    }
    auto solved_boxes = [&](const std::vector<uint16_t>& bx) {
        for (auto v : bx) if (!goal[v]) return false;
        return true;
    };
    // we need the real player position per state to rebuild a walk; keep it separately
    std::vector<uint16_t> real_player;
    real_player.push_back(idx(b.px, b.py));
    std::vector<int> push_dir_of(1, 0);

    for (size_t head = 0; head < states.size(); ++head) {
        if (states.size() > cap) return false;
        if (solved_boxes(states[head].boxes)) {
            // rebuild: list of (state index) chain, then walk between pushes
            std::vector<int> chain;
            for (int s = (int)head; s >= 0; s = states[s].parent) chain.push_back(s);
            std::reverse(chain.begin(), chain.end());
            moves.clear();
            std::vector<uint16_t> cur_boxes = states[0].boxes;
            uint16_t player = real_player[0];
            for (size_t k = 1; k < chain.size(); ++k) {
                const SState& ns = states[chain[k]];
                // find which push produced ns from cur_boxes: stored in real_player/push_dir_of
                int d = push_dir_of[chain[k]];
                uint16_t after = real_player[chain[k]];      // player cell after the push
                int ax = after % W - dx[d], ay = after / W - dy[d];
                uint16_t behind = idx(ax, ay);               // where the player stood to push
                std::vector<int> dist, from;
                reach(cur_boxes, player, dist, from);
                std::vector<Dir> walk;
                for (int c = behind; c != player; c = from[c]) {
                    int pc = from[c];
                    int ddx = c % W - pc % W, ddy = c / W - pc / W;
                    for (int dd = 0; dd < 4; ++dd) if (dx[dd] == ddx && dy[dd] == ddy) walk.push_back((Dir)dd);
                }
                std::reverse(walk.begin(), walk.end());
                for (Dir w : walk) moves.push_back(w);
                moves.push_back((Dir)d);
                cur_boxes = ns.boxes;
                player = after;
            }
            return true;
        }
        // expand pushes. Use the state's real player position only for reachability (same region as norm).
        std::vector<uint16_t> bx = states[head].boxes;
        std::vector<int> dist, from;
        auto occ = reach(bx, real_player[head], dist, from);
        for (size_t bi = 0; bi < bx.size(); ++bi) {
            int x = bx[bi] % W, y = bx[bi] / W;
            for (int d = 0; d < 4; ++d) {
                int px = x - dx[d], py = y - dy[d];
                int tx = x + dx[d], ty = y + dy[d];
                if (px < 0 || py < 0 || px >= W || py >= H || tx < 0 || ty < 0 || tx >= W || ty >= H) continue;
                if (dist[idx(px, py)] < 0) continue;
                uint16_t t = idx(tx, ty);
                if (wall[t] || occ[t]) continue;
                std::vector<uint16_t> nb = bx;
                nb[bi] = t;
                std::sort(nb.begin(), nb.end());
                uint16_t npl = bx[bi];                   // player ends where the box was
                std::vector<int> d2, f2;
                reach(nb, npl, d2, f2);
                uint16_t norm = 0xFFFF;
                for (int i = 0; i < W * H; ++i) if (d2[i] >= 0) { norm = (uint16_t)i; break; }
                std::string k = key_of(nb, norm);
                if (seen.count(k)) continue;
                seen[k] = (int)states.size();
                states.push_back({nb, norm, (int)head, d});
                real_player.push_back(npl);
                push_dir_of.push_back(d);
            }
        }
    }
    return false;
}

static bool replay(const Board& b, const std::vector<Dir>& moves)
{
    Game g;
    g.start(b);
    for (Dir d : moves)
        if (!g.move(d)) return false;
    if (!g.solved()) return false;
    // everything undone must give the start position back
    int n = 0;
    while (g.undo()) ++n;
    return n == (int)moves.size() && same_board(g.board(), b) && g.moves() == 0 && g.pushes() == 0;
}

static void test_builtin_pack()
{
    Pack p;
    CHECK(p.parse_text(BUILTIN_PACK_TEXT, strlen(BUILTIN_PACK_TEXT)));
    CHECK(p.count() == 6);
    CHECK(p.skipped() == 0);
    CHECK(strcmp(p.set_name(), "Initiation") == 0);
    char t[64];
    p.get_title(0, t, sizeof t);
    CHECK(strcmp(t, "First push") == 0);
    for (int i = 0; i < p.count(); ++i) {
        Board b;
        CHECK(p.get_board(i, b));
        std::vector<Dir> mv;
        bool ok = solve(b, mv);
        if (!ok) fprintf(stderr, "builtin level %d: no solution found\n", i + 1);
        CHECK(ok);
        if (ok) { CHECK(replay(b, mv)); }
    }
}

static void test_pack_text()
{
    const char* txt =
        "Date of Last Change:\n"
        "Set:       Demo Set\n"
        "Author:    Jane Doe\n"
        "This is free text with a # sign.\n"
        "\n"
        "1\n"
        "#####\n"
        "#@$.#\n"
        "#####\n"
        "Title: One\n"
        "Author: Someone Else\n"
        "Comment: first line\n"
        "second line\n"
        "Comment-End:\n"
        "\n"
        "; 2\n"
        "#####\n"
        "#@$.#\n"
        "#####\n"
        "; 3 (no blank line before this comment)\n"
        "###\n"
        "#@#\n"
        "###\n"            // invalid: no box
        "\n"
        "#####\n"
        "#@$.#\n"
        "#####";           // no trailing newline
    Pack p;
    CHECK(p.parse_text(txt, strlen(txt)));
    CHECK(p.count() == 3);
    CHECK(p.skipped() == 1);
    CHECK(strcmp(p.set_name(), "Demo Set") == 0);
    CHECK(strcmp(p.author(), "Jane Doe") == 0);
    char t[128];
    p.get_title(0, t, sizeof t);   CHECK(strcmp(t, "One") == 0);
    p.get_author(0, t, sizeof t);  CHECK(strcmp(t, "Someone Else") == 0);
    p.get_author(1, t, sizeof t);  CHECK(strcmp(t, "Jane Doe") == 0);      // falls back to the pack author
    p.get_comment(0, t, sizeof t); CHECK(strcmp(t, "first line second line") == 0);
    p.get_title(1, t, sizeof t);   CHECK(t[0] == '\0');
    Board b;
    CHECK(p.get_board(2, b) && b.w == 5);
    CHECK(!p.get_board(3, b) && !p.get_board(-1, b));

    // Latin-1 input is converted to UTF-8
    const char latin1[] = "Set: Caf\xE9\nAuthor: Ren\xE9 \xA9\n\n#####\n#@$.#\n#####\n";
    CHECK(!is_valid_utf8(latin1, sizeof(latin1) - 1));
    CHECK(p.parse_text(latin1, sizeof(latin1) - 1));
    CHECK(strcmp(p.set_name(), "Caf\xC3\xA9") == 0);
    CHECK(strcmp(p.author(), "Ren\xC3\xA9 \xC2\xA9") == 0);
    // ... and valid UTF-8 is left alone
    const char utf8[] = "Set: Caf\xC3\xA9\n\n#####\n#@$.#\n#####\n";
    CHECK(is_valid_utf8(utf8, sizeof(utf8) - 1));
    CHECK(p.parse_text(utf8, sizeof(utf8) - 1));
    CHECK(strcmp(p.set_name(), "Caf\xC3\xA9") == 0);

    CHECK(!p.parse_text("just some text\nno level here\n", 29));
    CHECK(!p.parse_text("", 0));
}

// ------------------------------------------------------------ real packs
static std::vector<std::string> list_sok(const char* dir)
{
    std::vector<std::string> out;
    DIR* d = opendir(dir);
    if (!d) return out;
    while (dirent* e = readdir(d)) {
        std::string n = e->d_name;
        if (n.size() > 4 && n.compare(n.size() - 4, 4, ".sok") == 0) out.push_back(n);
    }
    closedir(d);
    std::sort(out.begin(), out.end());
    return out;
}

// Prints "name count skipped" per pack (compared with tests/pack_counts.py by run_tests.sh),
// then solves the first levels of every pack to prove parser + rules agree with real puzzles.
static void test_real_packs(const char* dir, int per_pack, size_t cap)
{
    int solved_total = 0, tried_total = 0;
    for (const std::string& n : list_sok(dir)) {
        Pack p;
        std::string path = std::string(dir) + "/" + n;
        bool ok = p.load_file(path.c_str());
        CHECK(ok);
        printf("PACK %s %d\n", n.c_str(), p.count());
        if (!ok) continue;
        int solved = 0, tried = 0;
        for (int i = 0; i < p.count() && tried < per_pack; ++i) {
            Board b;
            CHECK(p.get_board(i, b));
            if (b.boxes > 6 || (int)b.w * b.h > 150) continue;     // keep the BFS small
            ++tried;
            std::vector<Dir> mv;
            if (solve(b, mv, cap)) {
                ++solved;
                bool r = replay(b, mv);
                if (!r) fprintf(stderr, "%s level %d: solution does not replay\n", n.c_str(), i + 1);
                CHECK(r);
            }
        }
        solved_total += solved; tried_total += tried;
        fprintf(stderr, "  %-34s levels %4d, solved %d of %d tried\n", n.c_str(), p.count(), solved, tried);
    }
    fprintf(stderr, "solver: %d of %d tried levels solved and replayed\n", solved_total, tried_total);
    if (!list_sok(dir).empty()) CHECK(solved_total >= 20);
}

// `engine_test --solve < board.txt` : tells whether a board (read from stdin) is solvable.
static int solve_stdin()
{
    std::string text, line;
    char buf[256];
    while (fgets(buf, sizeof buf, stdin)) text += buf;
    Board b;
    ParseStatus st = parse_board(text.data(), text.size(), b);
    if (st != ParseStatus::Ok) { printf("invalid: %s\n", parse_status_name(st)); return 2; }
    std::vector<Dir> mv;
    if (!solve(b, mv)) { printf("no solution found\n"); return 1; }
    printf("solved in %zu moves (%s)\n", mv.size(), replay(b, mv) ? "replay ok" : "REPLAY FAILED");
    return 0;
}

int main(int argc, char** argv)
{
    if (argc > 1 && strcmp(argv[1], "--solve") == 0) return solve_stdin();
    test_parse();
    test_rules();
    test_fuzz();
    test_history_limit();
    test_pack_text();
    test_builtin_pack();
    if (argc > 1) test_real_packs(argv[1], argc > 2 ? atoi(argv[2]) : 8, 150000);
    fprintf(stderr, "%d checks, %d failed\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
