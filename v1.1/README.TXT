Wave Audio File Playback Application for GNU-Linux Systems.
Version 1.1

Supported formats are mono and stereo, 16bit and 24bit. Sample rate compatibility depends on your audio hardware.

This code has several .cpp files. Each one of them generates an individual executable. "main.cpp" generates the main executable, which should be called by user to start the program. The other executables will be called from "main" as necessary.

When compiling, all executables except "main" should be named as their source files. Example: executable from "pb_16bit2ch.cpp" should be named "pb_16bit2ch.elf". If executable names (apart from "main") don't match their source file names, user will need to make changes on "main.cpp" in order to make the code run properly.

When compiling, two resources must be explicitly linked: -lasound and -lpthread

v1.1 Update:
"pb_params.h" file no longer exists. Instead, user shall pass output audio device id and input audio file directory as arguments in command line.

Author: Rafael Sabe 
Email: rafaelmsabe@gmail.com

