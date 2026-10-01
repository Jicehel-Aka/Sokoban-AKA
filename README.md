# Sokoban pour Gamebuino AKA (et PC / Linux / Windows)

Portage de **[Sokoban (GP2X)](https://github.com/joyrider3774/Sokoban)** de Willems Davy
(joyrider3774, licence MIT) sur la console **Gamebuino AKA**, avec une version **SDL2** qui fait
tourner **exactement le même code** sur PC.

- 19 packs de niveaux `.sok` (1963 niveaux) + 6 niveaux d'initiation intégrés ; on débloque un niveau
  en réussissant le précédent (option « tous les niveaux » pour tout ouvrir).
- Annuler (jusqu'à 1000 coups), refaire, recommencer, déplacement fluide, aperçu du niveau, sauvegarde.
- Musique (4 morceaux) et bruitages, 5 langues (FR, EN, DE, ES, IT), capture d'écran.
- **Éditeur de niveaux** (Titre → Éditeur) : dessine tes niveaux sur une grille 26 × 15, teste-les, enregistre-les
  dans `SOKOBAN/mylevels/MYPACKn.sok` ; ils apparaissent ensuite dans la liste des packs. Les packs fournis ne sont jamais modifiés.
- Les packs de niveaux sont de simples fichiers `.sok` : ajoute les tiens dans `SOKOBAN/levelpacks/`
  (de nombreux packs sur https://www.sokobano.de/en/levels.php). Limite : 26 × 15 cases.

## Commandes

| Touche console | Clavier PC | Action |
|---|---|---|
| Croix / joystick | flèches | se déplacer / naviguer |
| A | Entrée, Espace, Z | valider ; en jeu : **refaire** |
| B | Retour arrière, X | retour ; en jeu : **annuler** |
| C | C | en jeu : recommencer le niveau |
| L1 / R1 | A, R (ou PgSuiv / PgPréc) | piste précédente / suivante ; ±5 dans la liste des niveaux |
| MENU | Échap, M | menu pause (éditeur : retour au dessin). Maintenu 1 s : capture d'écran (`SHOTxxxx.BMP`) |
| RUN + MENU (0,5 s) | Maj + Échap | retour au loader (PC : quitte) |
| | F11 / F12 / Ctrl+Q | plein écran / capture / quitter (PC) |

### Éditeur

| Touche | Action |
|---|---|
| Croix | déplacer le curseur |
| A (maintenu + croix : peindre) | poser la pièce choisie |
| C | effacer la case |
| L1 / R1 | pièce précédente / suivante (effacer, mur, cible, caisse, joueur) |
| D | tester le niveau (réussi → retour à l'éditeur) |
| B | menu : tester, enregistrer, quitter sans enregistrer, tout effacer |

Dans la liste des niveaux d'un pack perso : A édite (ou crée avec `+`), D deux fois supprime. Un niveau doit avoir
un joueur, au moins une caisse et autant ou plus de caisses que de cibles pour être enregistré.

## Installer sur la console

1. Récupère `sokoban-aka-sdcard-vX.Y.Z.zip` dans les [Releases](../../releases).
2. Copie le dossier `SOKOBAN` à la racine de la carte SD (il contient `firmware.bin`, `meta.json`,
   `Picture.png`, `screen.bmp`, `levelpacks/`, `sound/`, `music/`).
3. Lance-le depuis le loader de la AKA. Les réglages et la progression sont écrits dans ce même
   dossier (`CFG.DAT`, `PROGRESS.DAT`).

## Jouer sur PC

- **Windows** : dézipper `sokoban-pc-windows-x64-…zip`, lancer `sokoban.exe`.
- **Linux** : installer SDL2 (`sudo apt install libsdl2-2.0-0`), dézipper `sokoban-pc-linux-x64-…zip`,
  lancer `./sokoban`.
- Options : `--scale N` (taille de la fenêtre), `--fullscreen`, `--data DIR`, `--save DIR`, `--help`.
  Le dossier `SOKOBAN` est cherché à côté de l'exécutable.

## Compiler

```bash
# PC (Linux / MSYS2) -- nécessite SDL2 et CMake
cmake -S pc -B build-pc -DCMAKE_BUILD_TYPE=Release
cmake --build build-pc
python3 tools/make_sd_audio.py music        # génère la musique WAV (ffmpeg requis)
build-pc/sokoban --data SD_files/SOKOBAN

# tests (moteur + parcours scripté de l'interface)
tests/run_tests.sh build-pc/sokoban

# Console -- ESP-IDF 5.5.1
python3 tools/fix_gamebuino_case.py components/gamebuino   # une fois
idf.py set-target esp32s3 && idf.py build                  # donne build/sokoban.bin = firmware.bin
```

Le workflow `.github/workflows/release.yml` fait tout cela à chaque mise à jour de la branche
principale (firmware ESP-IDF, versions Linux et Windows, tests) puis publie une Release en
incrémentant la version mineure.

## Organisation

```
main/engine/   règles, analyse des niveaux .sok, historique d'annulation (sans matériel, testé)
main/          jeu : écrans, entrées, audio, sauvegarde, textes (5 langues)
pc/            couche SDL2 qui remplace uniquement gb_ll_* (écran, clavier, son) -> même rendu au pixel près
components/gamebuino/   bibliothèque Gamebuino-AKA (LGPL)
assets/        sprites et sons d'origine, musiques sources (.ogg) ; tools/ les convertit
SD_files/SOKOBAN/       ce qui va sur la carte SD
tests/         tests du moteur, de l'interface (scriptée), du backend PC
```

## Licences et remerciements

Jeu original © Willems Davy (MIT). Les graphismes, musiques, sons et niveaux appartiennent à leurs
auteurs et gardent leurs licences (CC BY, CC BY-SA, CC0…). Tout est détaillé dans
**[CREDITS.md](CREDITS.md)** ; les textes de licences sont dans `LICENSE` et `THIRD_PARTY/`.
