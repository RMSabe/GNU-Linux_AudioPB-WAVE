# GNU-Linux_AudioPB-WAVE
Audio Playback code for GNU-Linux systems.

This code allows user to play audio files in wave (.wav) format. 
Supported formats are mono and stereo, 16bit and 24bit. Sample rate compatibility depends on your audio hardware.

This code has several .cpp files. Each one of them generates an individual executable. 
"main.cpp" generates the main executable, which should be called by user to start the program. 
The other executables will be called from "main" as necessary.

When compiling, all executables except "main" should be named as their source files.
Example: executable from "pb_16bit2ch.cpp" should be named "pb_16bit2ch.elf".
If executable names (apart from "main") don't match their source file names, 
user will need to make changes on "main.cpp" in order to make the code run properly.

This code requires the ALSA build resources to compile. 
On Debian-based distros, these resources can be installed with "sudo apt install libasound2-dev"

When compiling, two resources must be explicitly linked: -lasound and -lpthread

v1.0:
"pb_params.h" file is the user settings file where user sets input file directory and output audio device.
All ".cpp" files except "main.cpp" only need to be compiled once.
"main.cpp" must be recompiled every time user edits the "pb_params.h" file.

v1.1:
"pb_params.h" file no longer exists. Instead, user shall pass output audio device id and input audio file directory as arguments in command line.

I'm not a professional software developer. I made this code just for fun. Don't expect professional performance from it.

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com
