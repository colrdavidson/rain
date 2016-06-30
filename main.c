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


SDL_Texture *config_tex(char *filename, SDL_Renderer *renderer) {
	SDL_Surface *tmp_bmp = SDL_LoadBMP(filename);
	SDL_SetColorKey(tmp_bmp, SDL_TRUE, SDL_MapRGB(tmp_bmp->format, 0, 0, 0));
	SDL_Texture *tmp_tex = SDL_CreateTextureFromSurface(renderer, tmp_bmp);
	SDL_FreeSurface(tmp_bmp);

	return tmp_tex;
}

u32 to_1d(u32 x, u32 y, u32 z, u32 x_max, u32 y_max) {
	return (z * x_max * y_max) + (y * x_max) + x;
}

void print_as_3d(u32 idx, u32 x_max, u32 y_max) {
	u32 z = idx / (x_max * y_max);
	u32 tmp_idx = idx - (z * x_max * y_max);
	printf("(%d, %d, %d)\n", tmp_idx % x_max, tmp_idx / x_max, z);
}

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

	SDL_Texture *wall_tex = config_tex("assets/wall.bmp", renderer);
	SDL_Texture *brick_tex = config_tex("assets/brick.bmp", renderer);
	SDL_Texture *grass_tex = config_tex("assets/grass.bmp", renderer);
	SDL_Texture *wood_wall_tex = config_tex("assets/wood_wall.bmp", renderer);
	SDL_Texture *door_tex = config_tex("assets/door.bmp", renderer);
	SDL_Texture *roof_tex = config_tex("assets/roof.bmp", renderer);

	FILE *fp = fopen("assets/map", "r");
	char *line = malloc(256);

	fgets(line, 256, fp);
	u32 map_width = atoi(strtok(line, " "));
	u32 map_height = atoi(strtok(NULL, " "));
	u32 map_depth = atoi(strtok(NULL, " "));
	printf("Map Size: %dx%dx%d\n", map_width, map_height, map_depth);

	u8 map[map_width * map_depth * map_height];
	memset(map, 0, sizeof(map));

	fgets(line, 256, fp);

	for (u8 z = 0; z < map_depth; z++) {
		for (u8 y = 0; y < map_height; y++) {
			u8 new_line = 1;
			for (u8 x = 0; x < map_width; x++) {
				char *bit;
				if (new_line == 1) {
					bit = strtok(line, " ");
					new_line = 0;
				} else {
					bit = strtok(NULL, " ");
				}

				u8 tile_id = atoi(bit);
				map[to_1d(x, y, z, map_width, map_height)] = tile_id;
			}
			fgets(line, 256, fp);
		}
		fgets(line, 256, fp);
	}

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
		SDL_QueryTexture(wall_tex, NULL, NULL, &box_width, &box_height);

		SDL_Rect dest;
		dest.w = box_width;
		dest.h = box_height;

		for (u32 z = 0; z < map_depth; z++) {
			for (u32 x = 0; x < map_width; x++) {
				for (u32 y = map_height; y > 0; y--) {
					dest.x = ((x + y) * 16);
					dest.y = ((x - y) * 8) + (screen_height / 2) - (16 * z);

					u8 tile_id = map[to_1d(x, map_height - y, z, map_width, map_height)];
					switch (tile_id) {
						case 1: {
							SDL_RenderCopy(renderer, wall_tex, NULL, &dest);
						} break;
						case 2: {
							SDL_RenderCopy(renderer, grass_tex, NULL, &dest);
						} break;
						case 3: {
							SDL_RenderCopy(renderer, brick_tex, NULL, &dest);
						} break;
						case 4: {
							SDL_RenderCopy(renderer, wood_wall_tex, NULL, &dest);
						} break;
						case 5: {
							SDL_RenderCopy(renderer, door_tex, NULL, &dest);
						} break;
						case 6: {
							SDL_RenderCopy(renderer, roof_tex, NULL, &dest);
						} break;
					}
				}
			}
		}

		SDL_RenderPresent(renderer);
	}

    SDL_DestroyTexture(wall_tex);
	SDL_DestroyTexture(wood_wall_tex);
    SDL_DestroyTexture(brick_tex);
    SDL_DestroyTexture(door_tex);
    SDL_DestroyTexture(grass_tex);
	SDL_DestroyTexture(roof_tex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
    return 0;
}
