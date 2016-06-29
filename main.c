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
	SDL_Surface *floor_bmp = SDL_LoadBMP("assets/sidewalk.bmp");
	SDL_Surface *wall_bmp = SDL_LoadBMP("assets/wall.bmp");
	SDL_SetColorKey(floor_bmp, SDL_TRUE, SDL_MapRGB(floor_bmp->format, 0, 0, 0));
	SDL_SetColorKey(wall_bmp, SDL_TRUE, SDL_MapRGB(wall_bmp->format, 0, 0, 0));
	SDL_Texture *floor_tex = SDL_CreateTextureFromSurface(renderer, floor_bmp);
	SDL_Texture *wall_tex = SDL_CreateTextureFromSurface(renderer, wall_bmp);
	SDL_FreeSurface(floor_bmp);
	SDL_FreeSurface(wall_bmp);


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
		SDL_QueryTexture(floor_tex, NULL, NULL, &box_width, &box_height);

		SDL_Rect dest;
		dest.w = box_width;
		dest.h = box_height;

		for (i32 x = 0; x < 25; x++) {
			for (i32 y = 25; y > 0; y--) {
				dest.x = ((x + y) * 16);
				dest.y = ((x - y) * 8) + (screen_height / 2);
				if ((x % 3) == 0) {
					SDL_RenderCopy(renderer, floor_tex, NULL, &dest);
				} else {
					SDL_RenderCopy(renderer, wall_tex, NULL, &dest);
				}
			}
		}

		SDL_RenderPresent(renderer);
	}

    SDL_DestroyTexture(floor_tex);
    SDL_DestroyTexture(wall_tex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
    return 0;
}
