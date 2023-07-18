all: main.c
	gcc -o emu main.c -L/opt/homebrew/lib -I/opt/homebrew/include/SDL2 -lSDL2

clean:
	rm emu