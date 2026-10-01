Musique de Sokoban (AKA)
========================

Les fichiers .wav de ce dossier sont generes a la publication (workflow GitHub "release")
a partir des .ogg d'origine du jeu de Willems Davy, convertis en WAV mono 16 bits 44,1 kHz
(le seul format que la bibliotheque audio de la AKA sait lire).

Pour ajouter ta propre musique : copie ici un fichier .wav mono, 16 bits, 44100 Hz
(par exemple avec :  ffmpeg -i musique.mp3 -ac 1 -ar 44100 -c:a pcm_s16le musique.wav ).
- title.wav est jouee sur l'ecran titre ;
- toutes les autres sont jouees en boucle pendant les parties (L1 / R1 : piste precedente / suivante).
25 pistes au maximum, noms de 39 caracteres au plus.

Morceaux fournis et licences (details dans CREDITS.md) :
  title.wav     "title" - migfus20 - CC BY 4.0
  puzzle3.wav   "Puzzle Game 3" - Eric Matyas (soundimage.org) - CC BY 4.0
  periwink.wav  "periwinkle" - axtoncrolley - CC BY-SA 3.0
  calmbgm.wav   "041415calmbgm" - syncopika - CC BY 3.0
