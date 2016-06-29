#include "SDL2/SDL.h"
#include <stdio.h>

typedef unsigned long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef long i64;
typedef int i32;
typedef short i16;
typedef char i8;

int main() {
    u16 screen_width = 640;
    u16 screen_height = 480;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow("Rain", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, 0);


	if (window == NULL) {
		printf("!window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	SDL_Surface *bmp_surface = SDL_LoadBMP("assets/sidewalk.bmp");
	SDL_SetColorKey(bmp_surface, SDL_TRUE, SDL_MapRGB(bmp_surface->format, 0, 0, 0));
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, bmp_surface);
	SDL_FreeSurface(bmp_surface);


	u8 running = 1;
    while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = 0;
			}
		}

		SDL_RenderClear(renderer);

		i32 box_width, box_height;
		SDL_QueryTexture(texture, NULL, NULL, &box_width, &box_height);

		SDL_Rect dest;
		dest.w = box_width;
		dest.h = box_height;

		for (i32 x = 0; x < 25; x++) {
			for (i32 y = 0; y < 25; y++) {
				dest.x = ((x + y) * 16);
				dest.y = ((x - y) * 8) + (screen_height / 2);
				SDL_RenderCopy(renderer, texture, NULL, &dest);
			}
		}

		SDL_RenderPresent(renderer);
	}

    SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
    return 0;
}
