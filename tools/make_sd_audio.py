#!/usr/bin/env python3
"""Builds the audio files of the SD package in the exact format gb_audio_track_wav needs:
canonical 44 byte header, PCM, mono, 16 bit, 44100 Hz.

  tools/make_sd_audio.py sounds  [DEST]   assets/original/sound/*.wav -> DEST/sound/*.wav
  tools/make_sd_audio.py music   [DEST]   assets/music/*.ogg          -> DEST/music/*.wav   (needs ffmpeg)

DEST defaults to SD_files/SOKOBAN. Music is generated at release time (CI) because it is ~40 MB of WAV.
Sources and licences: see CREDITS.md.
"""
import os, subprocess, sys, wave, array

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RATE = 44100

# output name -> source (music names are kept short and without spaces: the SD card is FAT)
MUSIC = {
    "title": "title.ogg",
    "puzzle3": "Puzzle Game 3.ogg",
    "periwink": "periwinkle.ogg",
    "calmbgm": "041415calmbgm.ogg",
}


def write_wav(path, samples):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with wave.open(path, "wb") as w:        # the wave module writes the plain 44 byte header
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(samples.tobytes())


def mono_from_wav(path):
    with wave.open(path, "rb") as w:
        ch, width, rate = w.getnchannels(), w.getsampwidth(), w.getframerate()
        raw = w.readframes(w.getnframes())
    assert width == 2 and rate == RATE, "%s: expected 16 bit / 44100 Hz" % path
    a = array.array("h")
    a.frombytes(raw)
    if sys.byteorder == "big":
        a.byteswap()
    if ch == 1:
        return a
    out = array.array("h", [0]) * (len(a) // ch)
    for i in range(len(out)):
        out[i] = sum(a[i * ch:(i + 1) * ch]) // ch
    return out


def sounds(dest):
    src = os.path.join(ROOT, "assets", "original", "sound")
    for f in sorted(os.listdir(src)):
        if f.endswith(".wav"):
            write_wav(os.path.join(dest, "sound", f), mono_from_wav(os.path.join(src, f)))
            print("sound", f)


def music(dest):
    src = os.path.join(ROOT, "assets", "music")
    for name, f in MUSIC.items():
        cmd = ["ffmpeg", "-v", "error", "-i", os.path.join(src, f), "-ac", "1", "-ar", str(RATE),
               "-f", "s16le", "-"]
        raw = subprocess.run(cmd, check=True, stdout=subprocess.PIPE).stdout
        a = array.array("h")
        a.frombytes(raw[: len(raw) // 2 * 2])
        write_wav(os.path.join(dest, "music", name + ".wav"), a)
        print("music", name, "%.1f s" % (len(a) / RATE))


if __name__ == "__main__":
    what = sys.argv[1] if len(sys.argv) > 1 else "sounds"
    dest = sys.argv[2] if len(sys.argv) > 2 else os.path.join(ROOT, "SD_files", "SOKOBAN")
    {"sounds": sounds, "music": music}[what](dest)
