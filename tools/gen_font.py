#!/usr/bin/env python3
"""
 gen_font.py — Genere les glyphes ACCENTUES manquants (mAKAdames)

 Le SDK ne fournit que font8x8_basic : 128 glyphes, ASCII pur. Pas un accent.
 Impossible d'ecrire "Debutant" en francais, "Konigin" en allemand, "Espanol"
 en espagnol.

 On NE TOUCHE PAS a la police du SDK. On genere une TABLE SUPPLEMENTAIRE, et le
 moteur de texte decode l'UTF-8 pour y piocher. Les sources restent donc lisibles
 ("Débutant" s'ecrit "Débutant").

 CONTRAINTE PHYSIQUE : 8x8 pixels. Un accent occupe 2 rangees. Il ne reste donc
 que 6 rangees pour la lettre. Sur une MINUSCULE, ca passe : le corps de 'e' ou
 'a' tient deja dans les rangees 2 a 7. Sur une MAJUSCULE, la lettre occupe les
 8 rangees et il n'y a pas de place — on la comprime, ce qui donne l'equivalent
 d'une petite capitale. Lisible, mais l'interface est ecrite en casse mixte
 justement pour l'eviter au maximum.
"""
import re, sys, os
import numpy as np
from PIL import Image

SDK = 'components/gamebuino/include_lib/font8x8_basic.h'

def load_basic():
    txt = open(SDK, encoding='latin-1').read()
    rows = re.findall(r'\{\s*((?:0x[0-9A-Fa-f]{2}\s*,\s*){7}0x[0-9A-Fa-f]{2})\s*\}', txt)
    font = []
    for r in rows[:128]:
        font.append([int(v, 16) for v in re.findall(r'0x([0-9A-Fa-f]{2})', r)])
    return font

# Chaque glyphe = 8 octets, un par rangee. Le bit 0 est le pixel de GAUCHE
# (la police du SDK est en bit-reverse : row & (1 << x)).
def to_grid(g):
    return np.array([[(g[y] >> x) & 1 for x in range(8)] for y in range(8)], np.uint8)

def to_bytes(grid):
    return [int(sum(int(grid[y][x]) << x for x in range(8))) for y in range(8)]

# --- Accents ------------------------------------------------------------------
#
# METRIQUES REELLES de font8x8_basic, mesurees et non supposees :
#     minuscules : rangees 2..6  ->  les rangees 0 et 1 sont LIBRES
#     majuscules : rangees 0..6  ->  AUCUNE place au-dessus
#
# D'ou la decision : on n'accentue QUE les minuscules. Comprimer une majuscule de
# 7 rangees dans 5 pour degager l'accent donne une bouillie illisible (essaye et
# regarde : c'etait la premiere version). L'interface est donc ecrite en casse
# mixte — "Débutant", "Español", "Königin" — et les titres en capitales restent
# sans accent, ce qui est de toute facon l'usage typographique courant.
#
# Chaque accent doit rester DISTINGUABLE des autres en deux rangees de 8 pixels.
# Le piege : ´ et ` se confondent vite. On les decale donc lateralement — l'aigu
# monte vers la DROITE, le grave vers la GAUCHE.
ACCENTS = {
    'acute':  [(0, 4), (0, 5), (1, 3), (1, 4)],                   # ´ (monte a droite)
    'grave':  [(0, 2), (0, 3), (1, 3), (1, 4)],                   # ` (monte a gauche)
    'circ':   [(0, 3), (1, 2), (1, 4)],                           # ^
    'trema':  [(0, 2), (0, 5), (1, 2), (1, 5)],                   # ¨ (deux points nets)
    'tilde':  [(0, 2), (0, 3), (0, 5), (1, 2), (1, 4), (1, 5)],   # ~
    'cedille':[(7, 2), (7, 3)],                                   # SOUS la lettre (rangee 7,
                                                                  # libre chez les minuscules)
}

def compose(base_glyph, accent, dotless=False):
    """
    La lettre de base ne bouge pas (minuscule : rangees 2..6). L'accent se pose
    dans les rangees libres — 0 et 1 au-dessus, 7 en dessous pour la cedille.
    Aucune compression, aucune deformation.

    dotless : le 'i' est la seule minuscule qui occupe DEJA la rangee 0 (son
    point). Superpose a un accent, ca donne une tache indechiffrable — "Difícil"
    devient illisible. Toutes les vraies polices resolvent ca de la meme facon :
    elles retirent le point sous accent (le « i sans point », U+0131). On fait
    pareil.
    """
    out = to_grid(base_glyph).copy()
    if dotless:
        out[0][:] = 0
        out[1][:] = 0
    for (y, x) in ACCENTS[accent]:
        out[y][x] = 1
    return to_bytes(out)


# Caractere -> (lettre de base, accent). MINUSCULES UNIQUEMENT (cf. ci-dessus).
CHARS = [
    ('à','a','grave'), ('á','a','acute'), ('â','a','circ'), ('ä','a','trema'),
    ('ç','c','cedille'),
    ('è','e','grave'), ('é','e','acute'), ('ê','e','circ'), ('ë','e','trema'),
    # Le 'i' perd son point sous accent (cf. compose/dotless).
    ('ì','i','grave',1), ('í','i','acute',1), ('î','i','circ',1), ('ï','i','trema',1),
    ('ñ','n','tilde'),
    ('ò','o','grave'), ('ó','o','acute'), ('ô','o','circ'), ('ö','o','trema'),
    ('ù','u','grave'), ('ú','u','acute'), ('û','u','circ'), ('ü','u','trema'),
]

# Glyphes entierement dessines a la main. Chaque octet = une rangee ; le bit 0
# est le pixel de GAUCHE (la police du SDK est en bit-reverse).
def bits(*rows):
    """Convertit un dessin ASCII en octets. On DESSINE, on ne calcule pas en hexa."""
    out = []
    for r in rows:
        v = 0
        for x, ch in enumerate(r[:8]):
            if ch == '#':
                v |= 1 << x
        out.append(v)
    while len(out) < 8:
        out.append(0)
    return out

HAND = {
    # Eszett allemand. Il a une hampe : il monte jusqu'a la rangee 1.
    'ß': bits('........',
              '.####...',
              '##..##..',
              '##..##..',
              '#####...',
              '##..##..',
              '##..##..',
              '#####...'),
    # Point d'interrogation inverse (espagnol) : ouvre la phrase.
    '¿': bits('........',
              '..##....',
              '........',
              '..##....',
              '.##.....',
              '##......',
              '##..##..',
              '.####...'),
    '¡': bits('........',
              '..##....',
              '........',
              '..##....',
              '..##....',
              '..##....',
              '..##....',
              '........'),
}


def main(out_dir):
    basic = load_basic()
    glyphs = []          # (codepoint, 8 octets)

    for entry in CHARS:
        ch, base, acc = entry[0], entry[1], entry[2]
        dotless = (len(entry) > 3 and entry[3] == 1)
        glyphs.append((ord(ch), compose(basic[ord(base)], acc, dotless)))
    for ch, g in HAND.items():
        glyphs.append((ord(ch), g))

    glyphs.sort()

    # --- Apercu : on affiche de VRAIS MOTS ----------------------------------
    # Une grille de glyphes isoles ne dit rien. Ce qu'on veut savoir, c'est si un
    # mot se LIT — et si é se distingue de è, ce qui est tout l'enjeu en francais.
    table = {cp: g for cp, g in glyphs}
    WORDS = [
        "Debutant / Débutant",
        "eleve élève élevé",
        "Königin  Straße",
        "Español  ¿Difícil?",
        "Difficoltà  Città",
        "aàáâä eèéêë iìíîï",
        "oòóôö uùúûü çñ",
    ]
    Z = 5
    W = max(len(w) for w in WORDS) * 8
    H = len(WORDS) * 10
    img = np.zeros((H, W), np.uint8)
    for row, word in enumerate(WORDS):
        for i, ch in enumerate(word):
            cp = ord(ch)
            g = table.get(cp) if cp > 127 else basic[cp]
            if g is None:
                continue
            img[row*10:row*10+8, i*8:i*8+8] = to_grid(g) * 255
    Image.fromarray(img).resize((W*Z, H*Z), Image.NEAREST).save('/tmp/font.png')
    print(f"apercu -> /tmp/font.png  ({len(glyphs)} glyphes)")

    os.makedirs(out_dir, exist_ok=True)
    with open(os.path.join(out_dir, 'font_accents.h'), 'w') as f:
        f.write("/*\n  font_accents.h — GENERE par tools/gen_font.py, ne pas editer.\n"
                "  Glyphes accentues 8x8 absents de font8x8_basic (ASCII seul).\n"
                "  Recherche par point de code Unicode (table triee, dichotomie).\n*/\n"
                "#pragma once\n#include <cstdint>\n\n"
                "struct AccentGlyph { uint16_t cp; uint8_t rows[8]; };\n\n"
                f"constexpr int FONT_ACCENT_COUNT = {len(glyphs)};\n"
                "extern const AccentGlyph FONT_ACCENTS[FONT_ACCENT_COUNT];\n\n"
                "// Renvoie les 8 octets du glyphe, ou nullptr si le caractere est inconnu.\n"
                "const uint8_t* font_accent_lookup(uint16_t codepoint);\n")

    with open(os.path.join(out_dir, 'font_accents.cpp'), 'w') as f:
        f.write('#include "font_accents.h"\n\n')
        f.write("const AccentGlyph FONT_ACCENTS[FONT_ACCENT_COUNT] = {\n")
        for cp, g in glyphs:
            b = ",".join(f"0x{v:02X}" for v in g)
            f.write(f"  {{ 0x{cp:04X}, {{ {b} }} }},   // {chr(cp)}\n")
        f.write("};\n\n")
        f.write("""const uint8_t* font_accent_lookup(uint16_t cp) {
    // Table triee : dichotomie. Appele une fois par caractere affiche, donc
    // autant que ce soit en log(n).
    int lo = 0, hi = FONT_ACCENT_COUNT - 1;
    while (lo <= hi) {
        const int mid = (lo + hi) / 2;
        if (FONT_ACCENTS[mid].cp == cp) return FONT_ACCENTS[mid].rows;
        if (FONT_ACCENTS[mid].cp <  cp) lo = mid + 1;
        else                            hi = mid - 1;
    }
    return nullptr;
}
""")
    print(f"genere -> {out_dir}/font_accents.{{h,cpp}}")


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else 'main/assets/gfx')
