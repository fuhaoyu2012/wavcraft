CC=gcc
CFLAGS=-Wall -Wextra -ggdb -std=c11 -I/mingw64/include
LIBS=-L/mingw64/lib -lespeak-ng -lwinmm
DLL=/mingw64/bin/libespeak-ng.dll
te: main.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
	cp $(DLL) .
	cp /mingw64/bin/libpcaudio-0.dll .
	cp /mingw64/bin/libwinpthread-1.dll .
	cp /mingw64/bin/libgcc_s_seh-1.dll .
	cp /mingw64/bin/libstdc++-6.dll .
clean:
	rm -rf *.exe *.wav *.dll
