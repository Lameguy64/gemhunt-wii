# Gemhunt - Wii

This repository contains the source and most data files for a My Little Pony fan-game that I wasn't able to finish. As the name suggests, this is a Nintendo Wii homebrew game and is probably the first MLP fan-game to be developed for such a platform.

The source code of this project have been released mostly as learning materials for other fan-game developers to learn from if they wish to learn how to do homebrew game development for the Nintendo Wii. However, fans may continue development of this project but permission is required.

Pretty much everything needed is included to compile and run this game. The only files that we're not included are the music files (which are just mod music files from modarchive.org converted to ogg) to avoid copyright problems just in case.

The game requires a Classic Controller to play so modification of the source code is required to be able to play it with a Wiimote+Nunchuk combo.

Original Gemhunt project thread:
http://www.mylittlegamedev.com/thread-1284.html

# Precompiled Binaries

Pre-alpha Nintendo Wii version (works with Wiimote+Nunchuk):
http://lameguy64.github.io/gemhunt-wii/gemhunt-old-wii.zip

Pre-alpha PSP version (was an experimental port):
http://lameguy64.github.io/gemhunt-wii/gemhunt-old-psp.zip

PC (Win32) version:
http://lameguy64.github.io/gemhunt-wii/gemhunt-pc.zip

Please note that the PC version was built with the most recent source code of this game (it featured gem spawning and weapon equip slots) and was hastily ported over to PC at some point. The source code provided in this repository is not the same as the PC version but I'll be porting the PC source back to Wii eventually.

The reason why I'm not putting out the PC source right away is that the latest PC version of my MTek-GDL library (which the game uses) turned out awful compared to the Wii version of the library.

# Compiling

First off, you will need DevkitPPC which is part of the DevkitPro toolchain which you can download here:
https://sourceforge.net/projects/devkitpro/files/devkitPPC/

This game uses the latest version of the MTek-GDL library which you can get here:
https://github.com/Lameguy64/mtek-gdl

Use the version of the library provided in that repository because of major syntax changes and rewrites unlike the 7z releases provided in the wiibrew page.

Compile the game by either running the provided makefile or by opening the project through the CodeBlocks project file. The latter may require applying the compiler.conf file which is available in the most recent 7z release of MTek-GDL and apply it to your CodeBlocks IDE.