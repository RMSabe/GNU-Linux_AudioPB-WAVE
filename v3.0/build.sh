#!/bin/bash

g++ globldef.c -c -o globldef.o
g++ cstrdef.c -c -o cstrdef.o
g++ delay.c -c -o delay.o
g++ strdef.cpp -c -o strdef.o
g++ cppthread.cpp -c -o cppthread.o

g++ main.cpp -c -o main.o

g++ AudioPB.cpp -c -o AudioPB.o
g++ AudioPB_i16.cpp -c -o AudioPB_i16.o
g++ AudioPB_i24.cpp -c -o AudioPB_i24.o

g++ *.o -lpthread -lasound -o pb.elf

rm *.o

