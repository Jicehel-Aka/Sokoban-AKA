#!/usr/bin/env python3
"""Writes the scripted session that draws a tiny level in the editor, tests it, and saves it as MYPACK1.sok:

    #####
    #@$.#
    #####
"""
import sys

ev, f = [], 10
def tap(k, gap=6):
    global f
    ev.append("%d tap %s" % (f, k)); f += gap
def shot(n):
    global f
    ev.append("%d shot %s" % (f, n)); f += 2

tap("down"); tap("a", 14)               # title: Editor
tap("a", 14)                            # "+ new pack"
shot("ed_levels.bmp")
tap("a", 14)                            # "+" new level -> drawing board, cursor (13,7), part = wall
for _ in range(4): tap("a"); tap("right")
tap("a"); tap("down")                   # (17,7) wall -> (17,8)
tap("a"); tap("left")                   # wall at (17,8)
tap("r1"); tap("a"); tap("left")        # goal at (16,8)
tap("r1"); tap("a"); tap("left")        # box at (15,8)
tap("r1"); tap("a"); tap("left")        # player at (14,8)
tap("r1"); tap("r1"); tap("a"); tap("down")   # wall at (13,8), then (13,9)
for _ in range(4): tap("a"); tap("right")
tap("a")                                # (17,9)
shot("ed_draw.bmp")
tap("d", 20)                            # test the level
tap("right", 40)                        # push the box onto the goal
shot("ed_test.bmp")
tap("a", 14)                            # back to the editor
tap("b", 10)                            # editor menu
shot("ed_menu.bmp")
tap("down"); tap("down"); tap("a", 30)  # Save and exit
shot("ed_saved.bmp")
ev.append("%d quit" % (f + 5))
open(sys.argv[1], "w").write("\n".join(ev) + "\n")
