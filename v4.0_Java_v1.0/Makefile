buildso:
	g++ cppsrc/globldef.c -c -fPIC -o cppobj/globldef.o
	g++ cppsrc/delay.c -c -fPIC -o cppobj/delay.o
	g++ cppsrc/cstrdef.c -c -fPIC -o cppobj/cstrdef.o
	g++ cppsrc/strdef.cpp -c -fPIC -o cppobj/strdef.o
	g++ cppsrc/AudioPB.cpp -c -fPIC -o cppobj/AudioPB.o
	g++ cppsrc/AudioPB_i16.cpp -c -fPIC -o cppobj/AudioPB_i16.o
	g++ cppsrc/AudioPB_i24.cpp -c -fPIC -o cppobj/AudioPB_i24.o
	g++ cppsrc/libcore.cpp -c -fPIC -I${JDK_PATH}/include -I${JDK_PATH}/include/linux -o cppobj/libcore.o
	g++ cppobj/libcore.o cppobj/globldef.o cppobj/delay.o cppobj/cstrdef.o cppobj/strdef.o cppobj/AudioPB.o cppobj/AudioPB_i16.o cppobj/AudioPB_i24.o -lasound -shared -o libcore.so

buildj:
	${JDK_PATH}/bin/javac Definitions.java Core.java MyLayoutManager.java MyScreen.java Main.java ChooseFileScreen.java ChooseAudioDeviceScreen.java PlaybackRunningScreen.java PlaybackFinishedScreen.java

buildall: buildj buildso

run:
	${JDK_PATH}/bin/java -Djava.library.path=. Main

cleanup:
	rm cppobj/*.o

clearall: cleanup
	rm libcore.so
	rm *.class


