#!/usr/bin/env python3
"""Regenerates main/i18n.cpp from the table below and checks it against the enum in main/i18n.h.
Edit the table (one row per string: key, FR, EN, DE, ES, IT), keep i18n.h's enum in the same order, run:
    python3 tools/gen_i18n.py
UTF-8; only lowercase accents (the 8x8 font has no uppercase accented glyph); lines of <= 38 characters."""
import os, re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROWS = [
 ("S_PLAY",        "Jouer", "Play", "Spielen", "Jugar", "Gioca"),
 ("S_OPTIONS",     "Options", "Options", "Optionen", "Opciones", "Opzioni"),
 ("S_CREDITS",     "Crédits", "Credits", "Mitwirkende", "Créditos", "Crediti"),
 ("S_QUIT",        "Quitter", "Quit", "Beenden", "Salir", "Esci"),
 ("S_CHOOSE_PACK", "Choisis un pack", "Choose a pack", "Wähle ein Paket", "Elige un pack", "Scegli un pacchetto"),
 ("S_CHOOSE_LEVEL","Choisis un niveau", "Choose a level", "Wähle ein Level", "Elige un nivel", "Scegli un livello"),
 ("S_LEVELS_FMT",  "%d niveaux", "%d levels", "%d Level", "%d niveles", "%d livelli"),
 ("S_LEVEL_FMT",   "Niveau %d", "Level %d", "Level %d", "Nivel %d", "Livello %d"),
 ("S_MOVES",       "Coups", "Moves", "Züge", "Pasos", "Mosse"),
 ("S_PUSHES",      "Poussées", "Pushes", "Schübe", "Empujes", "Spinte"),
 ("S_LOCKED",      "Verrouillé", "Locked", "Gesperrt", "Bloqueado", "Bloccato"),
 ("S_SOLVED",      "Niveau terminé !", "Level complete!", "Level geschafft!", "¡Nivel completado!", "Livello completato!"),
 ("S_SOLVED_NEXT", "A : niveau suivant", "A: next level", "A: nächstes Level", "A: nivel siguiente", "A: livello successivo"),
 ("S_PACK_DONE",   "Pack terminé, bravo !", "Pack finished, well done!", "Paket gelöst, bravo!", "¡Pack terminado, bravo!", "Pacchetto finito, bravo!"),
 ("S_PAUSE",       "Pause", "Paused", "Pause", "Pausa", "Pausa"),
 ("S_RESUME",      "Reprendre", "Resume", "Weiter", "Continuar", "Riprendi"),
 ("S_RESTART",     "Recommencer", "Restart", "Neustart", "Reiniciar", "Ricomincia"),
 ("S_LEVEL_SELECT","Choix du niveau", "Level select", "Levelauswahl", "Elegir nivel", "Scelta livello"),
 ("S_TO_TITLE",    "Ecran titre", "Title screen", "Titelbild", "Pantalla de título", "Schermata titolo"),
 ("S_OPT_LANGUAGE","Langue", "Language", "Sprache", "Idioma", "Lingua"),
 ("S_OPT_MUSIC",   "Musique", "Music", "Musik", "Música", "Musica"),
 ("S_OPT_SFX",     "Bruitages", "Sound fx", "Effekte", "Efectos", "Effetti"),
 ("S_OPT_ANIM",    "Mouvement fluide", "Smooth moves", "Weiche Bewegung", "Movimiento suave", "Movimento fluido"),
 ("S_OPT_UNLOCK",  "Tous les niveaux", "All levels open", "Alle Level offen", "Todos los niveles", "Tutti i livelli"),
 ("S_ON",          "oui", "on", "an", "sí", "sì"),
 ("S_OFF",         "non", "off", "aus", "no", "no"),
 ("S_BACK",        "Retour", "Back", "Zurück", "Volver", "Indietro"),
 ("S_NO_PACKS",    "Aucun pack de niveaux", "No level pack found", "Keine Levelpakete", "Ningún pack de niveles", "Nessun pacchetto"),
 ("S_LOAD_ERROR",  "Pack illisible", "Pack unreadable", "Paket unlesbar", "Pack ilegible", "Pacchetto illeggibile"),
 ("S_SHOT_SAVED",  "Capture enregistrée", "Screenshot saved", "Bildschirmfoto gespeichert", "Captura guardada", "Schermata salvata"),
 ("S_SHOT_FAILED", "Capture impossible", "Screenshot failed", "Bildschirmfoto fehlgeschlagen", "Captura imposible", "Schermata non riuscita"),
 ("S_BUILTIN_HINT","(niveaux d'initiation)", "(starter levels)", "(Einsteiger-Level)", "(niveles de iniciación)", "(livelli iniziali)"),
 ("S_HELP_GAME",   "B:annuler A:refaire C:recommencer", "B:undo A:redo C:restart", "B:zurück A:vor C:neu", "B:deshacer A:rehacer C:reiniciar", "B:annulla A:ripeti C:ricomincia"),
 ("S_HELP_MENU",   "A:valider B:retour", "A:select B:back", "A:wählen B:zurück", "A:aceptar B:volver", "A:ok B:indietro"),
 ("S_HELP_LEVELS", "A:jouer B:retour L/R:+-5", "A:play B:back L/R:+-5", "A:los B:zurück L/R:+-5", "A:jugar B:volver L/R:+-5", "A:gioca B:indietro L/R:+-5"),
 ("S_QUIT_CONFIRM","Quitter le jeu ?", "Quit the game?", "Spiel beenden?", "¿Salir del juego?", "Uscire dal gioco?"),
 ("S_YES",         "Oui", "Yes", "Ja", "Sí", "Sì"),
 ("S_NO",          "Non", "No", "Nein", "No", "No"),
 ("S_CR_TITLE_GAME","Jeu d'origine", "Original game", "Originalspiel", "Juego original", "Gioco originale"),
 ("S_CR_TITLE_ART","Graphismes", "Graphics", "Grafik", "Gráficos", "Grafica"),
 ("S_CR_TITLE_SOUND","Musique et sons", "Music and sound", "Musik und Klang", "Música y sonido", "Musica e suoni"),
 ("S_CR_TITLE_LEVELS","Niveaux", "Levels", "Level", "Niveles", "Livelli"),
 ("S_CR_TITLE_PORT","Portage AKA", "AKA port", "AKA-Portierung", "Versión AKA", "Porting AKA"),
 ("S_PAGE_FMT",    "%d/%d", "%d/%d", "%d/%d", "%d/%d", "%d/%d"),
 # ---- level editor
 ("S_EDITOR",      "Editeur de niveaux", "Level editor", "Leveleditor", "Editor de niveles", "Editor di livelli"),
 ("S_ED_PACKS_TITLE","Mes packs (éditeur)", "My packs (editor)", "Meine Pakete (Editor)", "Mis packs (editor)", "I miei pacchetti (editor)"),
 ("S_ED_NEW_PACK", "Nouveau pack", "New pack", "Neues Paket", "Nuevo pack", "Nuovo pacchetto"),
 ("S_ED_NEW_LEVEL","Nouveau niveau", "New level", "Neues Level", "Nuevo nivel", "Nuovo livello"),
 ("S_ED_TEST",     "Tester le niveau", "Test the level", "Level testen", "Probar el nivel", "Prova il livello"),
 ("S_ED_SAVE",     "Enregistrer et quitter", "Save and exit", "Speichern und beenden", "Guardar y salir", "Salva ed esci"),
 ("S_ED_DISCARD",  "Quitter sans enregistrer", "Exit without saving", "Ohne Speichern beenden", "Salir sin guardar", "Esci senza salvare"),
 ("S_ED_CLEAR",    "Tout effacer", "Clear all", "Alles löschen", "Borrar todo", "Cancella tutto"),
 ("S_ED_SAVED",    "Pack enregistré", "Pack saved", "Paket gespeichert", "Pack guardado", "Pacchetto salvato"),
 ("S_ED_SAVE_FAILED","Ecriture impossible", "Cannot write the file", "Schreiben fehlgeschlagen", "No se pudo escribir", "Scrittura impossibile"),
 ("S_ED_DELETE_CONFIRM","D encore : supprimer", "Press D again to delete", "D nochmal: löschen", "D otra vez: borrar", "D ancora: elimina"),
 ("S_ED_TEST_OK",  "Test réussi !", "Test passed!", "Test bestanden!", "¡Prueba superada!", "Test superato!"),
 ("S_ED_ERASE",    "Effacer", "Erase", "Radieren", "Borrar", "Cancella"),
 ("S_ED_WALL",     "Mur", "Wall", "Wand", "Muro", "Muro"),
 ("S_ED_GOAL",     "Cible", "Goal", "Ziel", "Meta", "Obiettivo"),
 ("S_ED_BOX",      "Caisse", "Box", "Kiste", "Caja", "Cassa"),
 ("S_ED_PLAYER",   "Joueur", "Player", "Spieler", "Jugador", "Giocatore"),
 ("S_ED_HELP",     "A:pose C:efface L/R:pièce B:menu D:test", "A:put C:erase L/R:part B:menu D:test", "A:setzen C:weg L/R:Teil B:Menü D:Test", "A:poner C:borra L/R:pieza B:menú D:test", "A:metti C:canc L/R:pezzo B:menu D:test"),
 ("S_ED_LIST_HELP","A:éditer D x2:supprimer B:retour","A:edit D x2:delete B:back","A:ändern D x2:löschen B:zurück","A:editar D x2:borrar B:volver","A:modifica D x2:elimina B:indietro"),
 ("S_ED_PACK_TOO_BIG","Pack trop gros pour l'éditeur", "Pack too big for the editor", "Paket zu groß für den Editor", "Pack demasiado grande", "Pacchetto troppo grande"),
 ("S_ST_EMPTY",    "Il faut des murs", "Walls are missing", "Wände fehlen", "Faltan muros", "Mancano i muri"),
 ("S_ST_TOOBIG",   "Niveau trop grand", "Level too big", "Level zu groß", "Nivel demasiado grande", "Livello troppo grande"),
 ("S_ST_NOPLAYER", "Il manque le joueur", "The player is missing", "Spieler fehlt", "Falta el jugador", "Manca il giocatore"),
 ("S_ST_MANYPLAYERS","Un seul joueur !", "Only one player!", "Nur ein Spieler!", "¡Un solo jugador!", "Un solo giocatore!"),
 ("S_ST_NOBOX",    "Il manque des caisses", "Boxes are missing", "Kisten fehlen", "Faltan cajas", "Mancano le casse"),
 ("S_ST_NOGOAL",   "Il manque des cibles", "Goals are missing", "Ziele fehlen", "Faltan metas", "Mancano gli obiettivi"),
 ("S_ST_MISMATCH", "Plus de cibles que de caisses", "More goals than boxes", "Mehr Ziele als Kisten", "Más metas que cajas", "Più obiettivi che casse"),
]

def c(s):
    return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'

def main():
    hdr = open(os.path.join(ROOT, "main", "i18n.h")).read()
    names = re.findall(r'\bS_[A-Z_]+', hdr.split('enum Str {')[1].split('S_COUNT')[0])
    keys = [r[0] for r in ROWS]
    assert names == keys, "i18n.h enum and table differ: %s" % (set(names) ^ set(keys))
    for r in ROWS:
        assert len(r) == 6, r
        for s in r[1:]:
            assert "\n" not in s
    out = ['// GENERATED by tools/gen_i18n.py - edit the table there, not this file.',
           '// UTF-8, lowercase accents only (the 8x8 font has no uppercase accented glyph).',
           '#include "i18n.h"', '', 'namespace i18n {', '', 'static int s_lang = FR;', '',
           'static const char* const T[LANG_COUNT][S_COUNT] = {']
    for li, lang in enumerate(['FR', 'EN', 'DE', 'ES', 'IT']):
        out.append('  {  // ' + lang)
        for r in ROWS:
            out.append('    ' + c(r[1 + li]) + ',  // ' + r[0])
        out.append('  },')
    out += ['};', '',
            'static const char* const NAMES[LANG_COUNT] = { "Français", "English", "Deutsch", "Español", "Italiano" };', '',
            'void set_lang(int l) { s_lang = (l >= 0 && l < LANG_COUNT) ? l : FR; }',
            'int lang() { return s_lang; }',
            'const char* lang_name(int l) { return (l >= 0 && l < LANG_COUNT) ? NAMES[l] : NAMES[EN]; }',
            'const char* tr(Str s) { return (s >= 0 && s < S_COUNT) ? T[s_lang][s] : ""; }', '',
            '}  // namespace i18n', '']
    open(os.path.join(ROOT, "main", "i18n.cpp"), "w").write("\n".join(out))
    print(len(ROWS), "strings x 5 languages")

main()
