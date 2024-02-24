# AudioAnalyzer
VST3 AU LV2 Mac Windows Linux audio analyzer (Spectrogram Sonogram) plugin (JUCE)

## Known Issues
- Sometimes, but very rarely, crushing
- Use only one plugin instance. More instance glitching, crushing.

![screenshot1](spectrogram.png  "Spectrogram")


![screenshot2](sonogram.png  "Sonogram")


![screenshot3](options.png  "Options")



## Linux Build (Arch Linux)

sudo pacman -S base-devel juce

Launch app "Projucer" and open "AudioAnalyzer.jucer" 

Select exporter "Linux Makefile" and "File -> Save"

cd (download path)/AudioAnalyzer/Builds/LinuxMakefile

make Standalone CONFIG=Release

make VST3 CONFIG=Release

make LV2 CONFIG=Release

make clean (not necessary)

