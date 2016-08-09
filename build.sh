clang -O2 -Wall `sdl2-config --cflags` `sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lSDL2_ttf -std=c99 src/main.c -o rain
