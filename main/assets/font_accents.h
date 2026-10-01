/*
  font_accents.h — GENERE par tools/gen_font.py, ne pas editer.
  Glyphes accentues 8x8 absents de font8x8_basic (ASCII seul).
  Recherche par point de code Unicode (table triee, dichotomie).
*/
#pragma once
#include <cstdint>

struct AccentGlyph { uint16_t cp; uint8_t rows[8]; };

constexpr int FONT_ACCENT_COUNT = 25;
extern const AccentGlyph FONT_ACCENTS[FONT_ACCENT_COUNT];

// Renvoie les 8 octets du glyphe, ou nullptr si le caractere est inconnu.
const uint8_t* font_accent_lookup(uint16_t codepoint);
