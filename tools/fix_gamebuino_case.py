#!/usr/bin/env python3
"""Rend une arborescence C/C++ compatible avec un système de fichiers sensible à la casse (Linux).

Pour chaque #include "fichier.h" dont le nom ne correspond pas EXACTEMENT à un fichier de l'arborescence
mais correspond à un fichier à la casse près, remplace le nom par le vrai nom (en minuscules pour la
bibliothèque gamebuino). Les en-têtes système / ESP-IDF (introuvables localement) ne sont pas touchés.
Les fins de ligne (CRLF) sont conservées.

Usage : fix_gamebuino_case.py <dossier> [--check]      (--check : n'écrit rien, code 1 s'il y a un écart)
Exemple : fix_gamebuino_case.py wargame_aka/components/gamebuino
"""
import os
import re
import sys

EXTS = ('.h', '.hpp', '.c', '.cpp')
INC = re.compile(rb'(^[ \t]*#[ \t]*include[ \t]+")([^"]+)(")', re.M)


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    check_only = '--check' in sys.argv
    if len(args) != 1:
        print(__doc__)
        return 2
    root = args[0]
    exact, lower = set(), {}
    paths = []
    for d, _, fs in os.walk(root):
        for f in fs:
            exact.add(f)
            lower.setdefault(f.lower(), set()).add(f)
            if f.endswith(EXTS):
                paths.append(os.path.join(d, f))

    changes = 0
    for p in sorted(paths):
        data = open(p, 'rb').read()

        def fix(m):
            nonlocal changes
            inc = m.group(2).decode('utf-8', 'replace')
            head, base = os.path.split(inc)
            if base in exact:
                return m.group(0)
            cands = lower.get(base.lower())
            if not cands or len(cands) != 1:
                return m.group(0)          # en-tête système ou ambigu : on ne touche pas
            real = next(iter(cands))
            new = (head + '/' + real) if head else real
            print('%s : "%s" -> "%s"' % (os.path.relpath(p, root), inc, new))
            changes += 1
            return m.group(1) + new.encode('utf-8') + m.group(3)

        out = INC.sub(fix, data)
        if out != data and not check_only:
            open(p, 'wb').write(out)
    print('%d inclusion(s) %s' % (changes, 'à corriger' if check_only else 'corrigée(s)'))
    return 1 if (check_only and changes) else 0


if __name__ == '__main__':
    sys.exit(main())
