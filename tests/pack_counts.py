#!/usr/bin/env python3
"""Independent level counter for .sok files, used to cross-check the C++ parser.

Usage: pack_counts.py <levelpacks dir>   ->  prints "PACK <file> <count>" per pack.
It shares no code with main/engine/sok_pack.cpp: a different language, a regex
instead of the hand written scanner. Rules: a level is a run of consecutive rows
made only of board characters and holding at least one '#'; it counts when its
bounding box fits in 26 x 15, it has exactly one player, at least one box and
goal, and no fewer boxes than goals (and at least 3 rows).
"""
import os, re, sys

ROW = re.compile(r'^[#@+$*. \-_]+$')

def count(path):
    raw = open(path, 'rb').read()
    text = raw.decode('latin-1')
    lines = text.replace('\r', '').split('\n')
    n = 0
    block = []
    def flush():
        nonlocal n, block
        if len(block) >= 3:
            rows = block
            xs = [i for r in rows for i, c in enumerate(r) if c not in ' -_']
            ys = [j for j, r in enumerate(rows) if any(c not in ' -_' for c in r)]
            if xs and ys:
                w = max(xs) - min(xs) + 1
                h = max(ys) - min(ys) + 1
                joined = ''.join(rows)
                players = joined.count('@') + joined.count('+')
                boxes = joined.count('$') + joined.count('*')
                goals = joined.count('.') + joined.count('*') + joined.count('+')
                if w <= 26 and h <= 15 and players == 1 and boxes >= 1 and goals >= 1 and boxes >= goals:
                    n += 1
        block = []
    for ln in lines:
        if ln and ROW.match(ln) and '#' in ln:
            block.append(ln)
        else:
            flush()
    flush()
    return n

if __name__ == '__main__':
    d = sys.argv[1]
    for f in sorted(os.listdir(d)):
        if f.endswith('.sok'):
            print('PACK %s %d' % (f, count(os.path.join(d, f))))
