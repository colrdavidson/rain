#ifndef GAME_H
#define GAME_H

typedef enum GameState {
	MainMenuState,
	RainGameState,
} GameState;

typedef struct Game {
	SDL_Window *window;
	SDL_Renderer *renderer;

	u32 screen_width;
	u32 screen_height;
	u32 dpi_ratio;

	f32 rescale_x;
	f32 rescale_y;

    u8 redraw_buffer;
	u16 *click_map;

	u8 running;
	u8 transition;

	GameState cur_state;
} Game;

Game *new_game(SDL_Window *window, SDL_Renderer *renderer, u32 screen_width, u32 screen_height, u32 dpi_ratio) {
	Game *g = malloc(sizeof(Game));

	g->window = window;
	g->renderer = renderer;
	g->screen_width = screen_width;
	g->screen_height = screen_height;
	g->dpi_ratio = dpi_ratio;
	g->redraw_buffer = true;
	g->running = true;
	g->transition = false;
	g->cur_state = MainMenuState;
	g->rescale_x = 1.0;
	g->rescale_y = 1.0;

	u32 click_map_size = g->screen_width * g->screen_height * sizeof(u16);
	g->click_map = malloc(click_map_size);
	memset(g->click_map, 0, click_map_size);

	return g;
}

void wipe_clickbuffer(Game *game, u64 value) {
	u32 click_map_size = game->screen_width * game->screen_height * sizeof(u16);
	game->click_map = realloc(game->click_map, click_map_size);
	memset(game->click_map, value, click_map_size);
}

void blit_surface_to_click_buffer(SDL_Surface *surface, SDL_Rect *screen_rel_rect, u16 *click_map, u32 screen_width, u32 screen_height, u32 tile_num) {
	u32 pchunk_ptr = 0;

	if ((screen_rel_rect->x >= (i32)screen_width && (screen_rel_rect->x + screen_rel_rect->w) >= (i32)screen_width) || (screen_rel_rect->y >= (i32)screen_height && (screen_rel_rect->y + screen_rel_rect->h) >= (i32)screen_height)) {
		return;
	}

	for (i32 i = 0; i < surface->w * surface->h; i++) {
		u32 pixel = ((u8 *)surface->pixels)[pchunk_ptr + 3] << 24 | ((u8 *)surface->pixels)[pchunk_ptr + 2] << 16 | ((u8 *)surface->pixels)[pchunk_ptr + 1] << 8 | ((u8 *)surface->pixels)[pchunk_ptr];
		if (pixel != 0) {
			Point pix_pos = oned_to_twod(i, surface->w);
			if ((screen_rel_rect->x + pix_pos.x < screen_width) && (screen_rel_rect->y + pix_pos.y < screen_height)) {
				u32 click_idx = twod_to_oned(screen_rel_rect->x + pix_pos.x, screen_rel_rect->y + pix_pos.y, screen_width);
				if (click_idx < screen_width * screen_height) {
					click_map[click_idx] = tile_num;
				}
			}
		}
		pchunk_ptr += 4;
	}
}

void blit_rect_to_click_buffer(SDL_Rect *screen_rel_rect, u16 *click_map, u32 screen_width, u32 screen_height, u32 tile_num) {
	for (u32 i = 0; i < screen_rel_rect->w * screen_rel_rect->h; i++) {
		Point pix_pos = oned_to_twod(i, screen_rel_rect->w);
		if ((screen_rel_rect->x + pix_pos.x < screen_width) && (screen_rel_rect->y + pix_pos.y < screen_height)) {
			u32 click_idx = twod_to_oned(screen_rel_rect->x + pix_pos.x, screen_rel_rect->y + pix_pos.y, screen_width);
			if (click_idx < screen_width * screen_height) {
				click_map[click_idx] = tile_num;
			}
		}
	}
}

#endif
