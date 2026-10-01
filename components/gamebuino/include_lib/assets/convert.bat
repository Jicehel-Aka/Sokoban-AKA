@echo off
echo convert all wav files in this older to embedded ressource.
echo WAV files names MUST be compatible with C++ variable names : no space, no special chars..
SET RESSOURCE_FILE="WAV_SYSTEM.h"

echo // do not edit, file auto generated > %RESSOURCE_FILE%

for %%f in (*.wav) do (
echo convert %%f to gb_wav_system_%%~nf ...
echo const uint8_t gb_wav_system_%%~nf[] = { >> %RESSOURCE_FILE%
Bin2Hex.exe "%%f" >> %RESSOURCE_FILE%
echo }; >> %RESSOURCE_FILE%
)
pause
