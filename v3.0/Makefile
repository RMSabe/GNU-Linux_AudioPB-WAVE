globldef.o: globldef.c
	g++ globldef.c -c -o globldef.o

cstrdef.o: cstrdef.c
	g++ cstrdef.c -c -o cstrdef.o

delay.o: delay.c
	g++ delay.c -c -o delay.o

strdef.o: strdef.cpp
	g++ strdef.cpp -c -o strdef.o

cppthread.o: cppthread.cpp
	g++ cppthread.cpp -c -o cppthread.o

lib_res: globldef.o cstrdef.o delay.o strdef.o cppthread.o

AudioPB.o: AudioPB.cpp
	g++ AudioPB.cpp -c -o AudioPB.o

AudioPB_i16.o: AudioPB_i16.cpp
	g++ AudioPB_i16.cpp -c -o AudioPB_i16.o

AudioPB_i24.o: AudioPB_i24.cpp
	g++ AudioPB_i24.cpp -c -o AudioPB_i24.o

audio_obj: AudioPB.o AudioPB_i16.o AudioPB_i24.o

main.o: main.cpp
	g++ main.cpp -c -o main.o

pb.elf: main.o audio_obj lib_res
	g++ *.o -lpthread -lasound -o pb.elf

clear:
	rm *.o

all: pb.elf clear


