Audio Playback application for GNU-Linux systems.
Version 4.0 (Interop C++ Java version 1.0)

This is a GUI version of the Audio Playback application, integrating Java GUI resources to the existing C/C++ codebase.

This application supports WAVE ".wav" files, 16bit and 24bit.
Sample rate and number of channels compatibility depends on your audio hardware.

Compilation notes:
2 resources must be explicitly linked: -lpthread and  -lasound (ALSA API build resources)
Install package libasound2-dev to obtain ALSA API build resources.

Environment Variables:
JDK_PATH: this path variable must be set to your Java Development Kit (JDK) folder.
Makefile will use this variable for compiling/running the code.

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com

