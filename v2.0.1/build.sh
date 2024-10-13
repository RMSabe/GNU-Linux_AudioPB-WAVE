#!/bin/bash

g++ main.cpp AudioPlayback.cpp AudioPlayback_16bit1ch.cpp AudioPlayback_16bit2ch.cpp AudioPlayback_24bit1ch.cpp AudioPlayback_24bit2ch.cpp -lasound -o playback.elf

