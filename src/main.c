#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_mixer.h"
#include "SDL2/SDL_ttf.h"
#include <stdio.h>

#include "common.h"
#include "tga.h"
#include "point.h"
#include "path.h"
#include "map.h"
#include "game.h"
#include "main_menu.h"
#include "rain_game.h"

int main() {
	i32 original_screen_width = 640;
	i32 original_screen_height = 480;
    i32 screen_width;
    i32 screen_height;

	SDL_Init(SDL_INIT_VIDEO);
	IMG_Init(IMG_INIT_PNG);
	Mix_Init(0);
	TTF_Init();
	srand(time(NULL));

	SDL_Window *window = SDL_CreateWindow("Rain", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, original_screen_width, original_screen_height, SDL_WINDOW_ALLOW_HIGHDPI);

	if (window == NULL) {
		printf("!window: %s\n", SDL_GetError());
		return 1;
	}

	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);

	//Mix_PlayMusic(music, -1);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_GetRendererOutputSize(renderer, &screen_width, &screen_height);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	f32 scale_ratio = (f32)screen_width / (f32)original_screen_width;

	Game *game = new_game(window, renderer, screen_width, screen_height, scale_ratio);
	MainMenu *main_menu = init_main_menu(game);
	RainGame *rain_game = init_rain_game(game);

	f32 current_time = (f32)SDL_GetTicks() / 60.0;
	f32 t = 0.0;

    while (game->running) {
		switch (game->cur_state) {
			case MainMenuState: {
                handle_main_menu_events(main_menu, game);
			} break;
			case RainGameState: {
				handle_rain_game_events(rain_game, game);
			} break;
		}

		f32 new_time = (f32)SDL_GetTicks() / 60.0;
		f32 dt = new_time - current_time;
		current_time = new_time;

		if (game->transition) {
			switch (game->cur_state) {
				case MainMenuState: {
					// Update menu
				} break;
				case RainGameState: {
					transition_rain_game(rain_game, game);
				} break;
			}

			game->transition = false;
		}

		switch (game->cur_state) {
			case MainMenuState: {
				// Update menu
			} break;
			case RainGameState: {
				update_map(rain_game, game, &t, dt);
			} break;
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		switch (game->cur_state) {
			case MainMenuState: {
				render_menu(main_menu, game, dt);
			} break;
			case RainGameState: {
				render_rain_game(rain_game, game);
			} break;
		}

		game->redraw_buffer = false;
		SDL_RenderPresent(renderer);
	}

	Image img;
    img.width = screen_width;
    img.height = screen_height;
    img.bytes_per_pixel = 2;
	img.data = (Color *)game->click_map;
	write_tga("test.tga", &img);

	IMG_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
    return 0;
}
