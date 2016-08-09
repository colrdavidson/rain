#ifndef RAIN_GAME_H
#define RAIN_GAME_H

#include "dyn_array.h"

void rescale_surfaces(SDL_Surface **surface_map, SDL_Surface **scaled_surface_map, u32 num_surfaces, f32 scale) {
	for (u32 i = 1; i < num_surfaces; i++) {
		SDL_Surface *scaled = SDL_CreateRGBSurface(0, (u32)((f32)surface_map[i]->w * scale), (u32)((f32)surface_map[i]->h * scale), surface_map[i]->format->BitsPerPixel, surface_map[i]->format->Rmask, surface_map[i]->format->Gmask, surface_map[i]->format->Bmask, surface_map[i]->format->Amask);
		SDL_BlitScaled(surface_map[i], NULL, scaled, NULL);

		SDL_FreeSurface(scaled_surface_map[i]);
		scaled_surface_map[i] = scaled;
	}
}

typedef struct Camera {
	Direction direction;
	f32 scale;

	f32 pos_x;
	f32 pos_y;
	f32 vel_x;
	f32 vel_y;
	f32 imp_x;
	f32 imp_y;
	f32 speed;
} Camera;

Camera *new_camera(f32 pos_x, f32 pos_y, f32 speed, f32 scale, Direction dir) {
	Camera *c = malloc(sizeof(Camera));
	c->direction = dir;
	c->scale = scale;
	c->pos_x = pos_x;
	c->pos_y = pos_y;
	c->vel_x = 0.0;
	c->vel_y = 0.0;
	c->imp_x = 0.0;
	c->imp_y = 0.0;
	c->speed = speed;

	return c;
}


typedef struct RainGame {
	Map *map;
	Camera *camera;

	Mix_Chunk *shoot_wav;
	Mix_Chunk *step_wav;
	Mix_Music *music;

	Entity **entity_map;
	u32 entity_map_length;
	u32 max_players;
	u32 max_enemies;

	u32 player_idx;
	u32 enemy_idx;

	u32 tile_entries;
	DynArray *surface_map;
	DynArray *scaled_surface_map;
	DynArray *texture_map;

	u32 max_neighbors;
	GridNode *node_map;

	PathNode *path;
	PathNode *cur_pos;

    Point selected_char;

	u8 player_turn;
	u8 travelling;
} RainGame;

void update_camera(Camera *camera, f32 friction, f32 dt) {
	f32 x_acc = friction * camera->vel_x + camera->imp_x;
	f32 y_acc = friction * camera->vel_y + camera->imp_y;
	camera->pos_x = (0.5 * x_acc * dt * dt) + camera->vel_x * dt + camera->pos_x;
	camera->pos_y = (0.5 * y_acc * dt * dt) + camera->vel_y * dt + camera->pos_y;
	camera->vel_x = (x_acc * dt) + camera->vel_x;
	camera->vel_y = (y_acc * dt) + camera->vel_y;
}

RainGame *init_rain_game(Game *game) {
	RainGame *r = malloc(sizeof(RainGame));

	r->shoot_wav = Mix_LoadWAV("assets/shoot.wav");
	r->step_wav = Mix_LoadWAV("assets/step.wav");
	r->music = Mix_LoadMUS("assets/rain.wav");

	FILE *fp = fopen("assets/house_map", "r");
	char *line = malloc(256);

	fgets(line, 256, fp);
	u32 map_width = atoi(strtok(line, " "));
	u32 map_height = atoi(strtok(NULL, " "));
	u32 map_depth = atoi(strtok(NULL, " ")) + 1;
	printf("Map Size: %dx%dx%d\n", map_width, map_height, map_depth);

	r->map = new_map(map_width, map_height, map_depth);
	wipe_clickbuffer(game, r->map->size);

	fgets(line, 256, fp);
	fgets(line, 256, fp);

	r->tile_entries = atoi(line) + 3;
	r->surface_map = new_dyn_array(r->tile_entries);
	r->scaled_surface_map = new_dyn_array(r->tile_entries);
	r->texture_map = new_dyn_array(r->tile_entries);

	r->player_idx = r->tile_entries - 1;
	r->enemy_idx = r->tile_entries - 2;
	for (u32 i = 1; i < r->tile_entries - 2; i++) {
		fgets(line, 256, fp);

		u32 idx = atoi(strtok(line, " "));
		char *img_filename = strtok(NULL, " ");
		img_filename[strlen(img_filename) - 1] = 0;

		r->surface_map->arr[idx] = IMG_Load(img_filename);
		if (r->surface_map->arr[idx] == NULL) {
			printf("Error opening tile %u, (%s)\n", idx, img_filename);
			return 0;
		}

		r->texture_map->arr[idx] = SDL_CreateTextureFromSurface(game->renderer, r->surface_map->arr[idx]);
	}

	r->surface_map->arr[r->player_idx] = IMG_Load("assets/player_cylinder.png");
	r->texture_map->arr[r->player_idx] = SDL_CreateTextureFromSurface(game->renderer, r->surface_map->arr[r->player_idx]);

	r->surface_map->arr[r->enemy_idx] = IMG_Load("assets/enemy_cylinder.png");
	r->texture_map->arr[r->enemy_idx] = SDL_CreateTextureFromSurface(game->renderer, r->surface_map->arr[r->enemy_idx]);

	r->camera = new_camera(-10.0, 250.0, 2.0, 1.0, NORTH);

	for (u32 i = 1; i < r->tile_entries; i++) {
		i32 tile_width, tile_height;
		SDL_QueryTexture(r->texture_map->arr[i], NULL, NULL, &tile_width, &tile_height);
		SDL_Surface *surface = SDL_CreateRGBSurface(0, (u32)tile_width, (u32)tile_height, ((SDL_Surface *)r->surface_map->arr[i])->format->BitsPerPixel, ((SDL_Surface *)r->surface_map->arr[i])->format->Rmask, ((SDL_Surface *)r->surface_map->arr[i])->format->Gmask, ((SDL_Surface *)r->surface_map->arr[i])->format->Bmask, ((SDL_Surface *)r->surface_map->arr[i])->format->Amask);
		SDL_BlitScaled(r->surface_map->arr[i], NULL, surface, NULL);
		r->scaled_surface_map->arr[i] = surface;
	}

	fgets(line, 256, fp);
	fgets(line, 256, fp);

	r->max_players = 4;
	r->max_enemies = 20;
    r->entity_map = malloc(sizeof(Entity *) * (r->max_players + r->max_enemies));
	u32 player_map_idx = 0;
	u32 enemy_map_idx = 0;

	for (u32 z = 0; z < r->map->depth - 1; z++) {
		for (u32 y = 0; y < r->map->height; y++) {
			u8 new_line = 1;
			for (u32 x = 0; x < r->map->width; x++) {
				char *bit;
				if (new_line == 1) {
					bit = strtok(line, " ");
					new_line = 0;
				} else {
					bit = strtok(NULL, " ");
				}

				if (strncmp(bit, "p", 1) == 0) {
					if (player_map_idx < r->max_players) {
						set_map_entity(r->map, x, y, z, new_entity(r->player_idx, 2, 4, r->camera->direction));
						r->entity_map[player_map_idx] = get_map_entity(r->map, x, y, z);
						player_map_idx++;
					} else {
						puts("[MAP ERROR] Too many players!");
						return NULL;
					}
				} else if (strncmp(bit, "e", 1) == 0) {
					if (enemy_map_idx < r->max_enemies) {
						set_map_entity(r->map, x, y, z, new_entity(r->enemy_idx, 2, 4, r->camera->direction));
						r->entity_map[r->max_players + enemy_map_idx] = get_map_entity(r->map, x, y, z);
						enemy_map_idx++;
					} else {
						puts("[MAP ERROR] Too many enemies!");
						return NULL;
					}
				} else {
					u32 tile_id = atoi(bit);
					if (tile_id) {
						set_map_tile(r->map, x, y, z, new_tile(tile_id), 1);
					}
				}
			}
			fgets(line, 256, fp);
		}
		fgets(line, 256, fp);
	}

	r->entity_map_length = player_map_idx + enemy_map_idx;

	r->player_turn = true;
	r->travelling = false;

	if (game->dpi_ratio != 1.0f) {
		r->camera->scale = game->dpi_ratio;
		rescale_surfaces(((SDL_Surface **)r->surface_map->arr), ((SDL_Surface **)r->scaled_surface_map->arr), r->tile_entries, r->camera->scale);
	}

	r->max_neighbors = 10;
	r->node_map = new_nodemap(r->map, r->max_neighbors);

	r->path = NULL;
	r->cur_pos = NULL;

    r->selected_char = new_point(0, 0, 0);
	return r;
}

void transition_rain_game(RainGame *rain_game, Game *game) {
	Camera *tmp_cam = rain_game->camera;
	rain_game->camera = new_camera(-10.0, 250.0, 2.0, tmp_cam->scale, tmp_cam->direction);
	free(tmp_cam);

	wipe_clickbuffer(game, rain_game->map->size);
	game->redraw_buffer = true;

	rain_game->camera->scale *= game->rescale_x;
	rescale_surfaces(((SDL_Surface **)rain_game->surface_map->arr), ((SDL_Surface **)rain_game->scaled_surface_map->arr), rain_game->tile_entries, rain_game->camera->scale);
}

void handle_rain_game_events(RainGame *rain_game, Game *game) {
	SDL_Event event;

	rain_game->camera->imp_x = 0.0;
	rain_game->camera->imp_y = 0.0;

	SDL_PumpEvents();
	const u8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_UP]) {
		rain_game->camera->imp_y += rain_game->camera->speed;
		game->redraw_buffer = true;
	}
	if (state[SDL_SCANCODE_DOWN]) {
		rain_game->camera->imp_y -= rain_game->camera->speed;
		game->redraw_buffer = true;
	}
	if (state[SDL_SCANCODE_LEFT]) {
		rain_game->camera->imp_x += rain_game->camera->speed;
		game->redraw_buffer = true;
	}
	if (state[SDL_SCANCODE_RIGHT]) {
		rain_game->camera->imp_x -= rain_game->camera->speed;
		game->redraw_buffer = true;
	}

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_KEYDOWN: {
				switch (event.key.keysym.sym) {
					case SDLK_e: {
						rain_game->camera->direction = cycle_right(rain_game->camera->direction);
						game->redraw_buffer = true;
					} break;
					case SDLK_ESCAPE: {
						game->cur_state = MainMenuState;
						game->transition = true;
					} break;
					case SDLK_q: {
						rain_game->camera->direction = cycle_left(rain_game->camera->direction);
						game->redraw_buffer = true;
					} break;
					case SDLK_1: {
						rain_game->camera->scale = 1.0;
						rescale_surfaces(((SDL_Surface **)rain_game->surface_map->arr), ((SDL_Surface **)rain_game->scaled_surface_map->arr), rain_game->tile_entries, rain_game->camera->scale);
						game->redraw_buffer = true;
					} break;
					case SDLK_2: {
						rain_game->camera->scale = 2.0;
						rescale_surfaces(((SDL_Surface **)rain_game->surface_map->arr), ((SDL_Surface **)rain_game->scaled_surface_map->arr), rain_game->tile_entries, rain_game->camera->scale);
						game->redraw_buffer = true;
					} break;
					case SDLK_3: {
						rain_game->camera->scale = 3.0;
						rescale_surfaces(((SDL_Surface **)rain_game->surface_map->arr), ((SDL_Surface **)rain_game->scaled_surface_map->arr), rain_game->tile_entries, rain_game->camera->scale);
						game->redraw_buffer = true;
					} break;
				}
			} break;
			case SDL_MOUSEBUTTONDOWN: {
				i32 mouse_x, mouse_y;
				u32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

				mouse_x = game->dpi_ratio * mouse_x;
				mouse_y = game->dpi_ratio * mouse_y;

				Point p = oned_to_threed(game->click_map[twod_to_oned(mouse_x, mouse_y, game->screen_width)], rain_game->map->width, rain_game->map->height);

				if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
					Point hover_p = new_point(p.x, p.y, p.z + 1);
					if (has_entity(rain_game->map, p.x, p.y, p.z) && get_map_entity(rain_game->map, p.x, p.y, p.z)->sprite_id == rain_game->player_idx && !rain_game->travelling) {
						rain_game->selected_char = p;
					} else if (rain_game->player_turn && can_move(rain_game->map, rain_game->selected_char, hover_p) && get_map_entity(rain_game->map, rain_game->selected_char.x, rain_game->selected_char.y, rain_game->selected_char.z)->sprite_id == rain_game->player_idx) {
						if (rain_game->path == NULL && rain_game->cur_pos == NULL) {
							Point start = rain_game->selected_char;
							Point goal = p;
							goal.z += 1;
							rain_game->path = find_path(start, goal, rain_game->node_map, rain_game->map->width, rain_game->map->height, rain_game->map->depth, rain_game->max_neighbors);
							if (rain_game->path != NULL) {
								get_map_entity(rain_game->map, rain_game->selected_char.x, rain_game->selected_char.y, rain_game->selected_char.z)->turn_points--;
								rain_game->cur_pos = rain_game->path->next;
								if (rain_game->cur_pos == NULL) {
									puts("cur_pos == NULL");
								}
								rain_game->travelling = true;
							}
						}
					}
				} else if (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
					if (rain_game->player_turn && has_entity(rain_game->map, p.x, p.y, p.z) && get_map_entity(rain_game->map, p.x, p.y, p.z)->sprite_id == rain_game->enemy_idx && has_entity(rain_game->map, rain_game->selected_char.x, rain_game->selected_char.y, rain_game->selected_char.z) && get_map_entity(rain_game->map, rain_game->selected_char.x, rain_game->selected_char.y, rain_game->selected_char.z)->sprite_id == rain_game->player_idx) {
						Entity *player = get_map_entity(rain_game->map, rain_game->selected_char.x, rain_game->selected_char.y, rain_game->selected_char.z);
						Entity *enemy = get_map_entity(rain_game->map, p.x, p.y, p.z);

						if (fire_at_entity(rain_game->map, rain_game->entity_map, rain_game->entity_map_length, player, enemy, rain_game->shoot_wav)) {
							game->redraw_buffer = true;
						}
					}
				}

			} break;
			case SDL_WINDOWEVENT: {
				switch (event.window.event) {
					case SDL_WINDOWEVENT_RESIZED: {
						printf("resized! %d, %d\n", event.window.data1, event.window.data2);
						i32 tmp_width;
						i32 tmp_height;
						SDL_GetRendererOutputSize(game->renderer, &tmp_width, &tmp_height);

						game->rescale_x = (f32)tmp_width / (f32)game->screen_width;
						game->rescale_y = (f32)tmp_height / (f32)game->screen_height;
						game->screen_width = tmp_width;
						game->screen_height = tmp_height;

						wipe_clickbuffer(game, rain_game->map->size);

						rain_game->camera->scale *= game->rescale_x;
						rescale_surfaces(((SDL_Surface **)rain_game->surface_map->arr), ((SDL_Surface **)rain_game->scaled_surface_map->arr), rain_game->tile_entries, rain_game->camera->scale);

						game->redraw_buffer = true;
					} break;
				}
			} break;
			case SDL_QUIT: {
				game->running = false;
			} break;
		}
	}
}

void update_map(RainGame *rain_game, Game *game, f32 *t, f32 dt) {
	update_camera(rain_game->camera, -0.2, dt);

	if (rain_game->cur_pos != NULL) {
		if (*t > 3.0) {
			Mix_PlayChannel(-1, rain_game->step_wav, 0);
			move_entity(rain_game->map, rain_game->selected_char, rain_game->cur_pos->p);
			rain_game->selected_char = rain_game->cur_pos->p;
			rain_game->cur_pos = rain_game->cur_pos->next;
			*t = 0.0;
			game->redraw_buffer = true;
		}
	} else {
		if (rain_game->path != NULL) {
			free_path(rain_game->path);
			rain_game->path = NULL;
			rain_game->travelling = false;
		}
	}
	*t += dt;

	if (game->redraw_buffer) {
		memset(game->click_map, rain_game->map->size, game->screen_width * game->screen_height * sizeof(u16));
	}

	u8 redraw_lights = false;
	for (u32 i = 0; i < rain_game->entity_map_length; i++) {
		if (rain_game->entity_map[i] != NULL && rain_game->entity_map[i]->moved == true) {
			redraw_lights = true;
		}
	}

	if (redraw_lights) {
		for (u32 i = 0; i < rain_game->map->size; i++) {
			rain_game->map->spaces[i]->visible = false;
		}

		for (u32 i = 0; i < rain_game->max_players; i++) {
			if (rain_game->entity_map[i] != NULL) {
				update_visible(rain_game->map, rain_game->entity_map[i], true);
				rain_game->entity_map[i]->moved = false;
			}
		}

		for (u32 i = rain_game->max_players; i < rain_game->entity_map_length; i++) {
			if (rain_game->entity_map[i] != NULL) {
				update_visible(rain_game->map, rain_game->entity_map[i], false);
				rain_game->entity_map[i]->moved = false;
			}
		}
	}


	if (!rain_game->player_turn) {
		for (u32 i = rain_game->max_players; i < rain_game->entity_map_length; i++) {
			if (rain_game->entity_map[i] != NULL) {
				SightNode *tmp = rain_game->entity_map[i]->vis_head;
				u8 found = false;
				while (tmp != NULL && !found) {
					for (u32 p_idx = 0; p_idx < rain_game->max_players && !found; p_idx++) {
						if (rain_game->entity_map[p_idx] != NULL && point_eq(rain_game->entity_map[p_idx]->pos, tmp->p)) {
							fire_at_entity(rain_game->map, rain_game->entity_map, rain_game->entity_map_length, rain_game->entity_map[i], rain_game->entity_map[p_idx], rain_game->shoot_wav);
							game->redraw_buffer = true;
							found = true;
						}
					}
					tmp = tmp->next;
				}
			}
		}

		for (u32 i = 0; i < rain_game->max_players; i++) {
			if (rain_game->entity_map[i] != NULL) {
				rain_game->entity_map[i]->turn_points = 2;
			}
		}

		rain_game->player_turn = true;
	}
}

void render_rain_game(RainGame *rain_game, Game *game) {
	for (u32 z = 0; z < rain_game->map->depth; z++) {
		for (u32 x = 0; x < rain_game->map->width; x++) {
			for (u32 y = 0; y < rain_game->map->height; y++) {

				f32 adj_y;
				f32 adj_x;

				f32 cam_adj_x;
				f32 cam_adj_y;

				switch (rain_game->camera->direction) {
					case NORTH: {
						adj_y = y;
						adj_x = x;

						cam_adj_x = x;
						cam_adj_y = rain_game->map->height - y;

						/*
						dir_door_bmp = north_door_bmp;
						dir_door_tex = north_door_tex;
						*/
					} break;
					case EAST: {
						adj_y = rain_game->map->height - y - 1;
						adj_x = x;

						cam_adj_x = y;
						cam_adj_y = rain_game->map->width - x;
					} break;
					case SOUTH: {
						adj_y = rain_game->map->height - y - 1;
						adj_x = rain_game->map->width - x - 1;

						cam_adj_x = x;
						cam_adj_y = rain_game->map->height - y;
					} break;
					case WEST: {
						adj_x = rain_game->map->width - x - 1;
						adj_y = y;

						cam_adj_x = y;
						cam_adj_y = rain_game->map->width - x;

						/*
						dir_door_bmp = west_door_bmp;
						dir_door_tex = west_door_tex;
						*/
					} break;
					default: {
						puts("direction wtf?");
						adj_y = y;
						adj_x = x;

						cam_adj_x = x;
						cam_adj_y = rain_game->map->height - y;

						/*
						dir_door_bmp = north_door_bmp;
						dir_door_tex = north_door_tex;
						*/
					} break;
				}

				if (has_tile(rain_game->map, adj_x, adj_y, z)) {
					u32 tile_id = get_map_tile(rain_game->map, adj_x, adj_y, z)->sprite_id;

					SDL_Rect dest;
					dest.w = (i32)(((f32)((SDL_Surface *)rain_game->surface_map->arr[tile_id])->w) * rain_game->camera->scale);
					dest.h = (i32)(((f32)((SDL_Surface *)rain_game->surface_map->arr[tile_id])->h) * rain_game->camera->scale);
					dest.x = (((cam_adj_x + cam_adj_y) * 16.0) + rain_game->camera->pos_x) * rain_game->camera->scale;
					dest.y = ((((cam_adj_x - cam_adj_y) * 8.0) - (16.0 * (f32)z) + rain_game->camera->pos_y) * rain_game->camera->scale) - dest.h;


					if (get_map_space(rain_game->map, adj_x, adj_y, z)->visible) {
						SDL_SetTextureColorMod(((SDL_Texture *)rain_game->texture_map->arr[tile_id]), 255, 255, 255);

						if (game->redraw_buffer) {
							blit_surface_to_click_buffer(((SDL_Surface *)rain_game->scaled_surface_map->arr[tile_id]), &dest, game->click_map, game->screen_width, game->screen_height, threed_to_oned(adj_x, adj_y, z, rain_game->map->width, rain_game->map->height));
						}
					} else {
						SDL_SetTextureColorMod(((SDL_Texture *)rain_game->texture_map->arr[tile_id]), 100, 100, 100);
					}
					SDL_RenderCopy(game->renderer, ((SDL_Texture *)rain_game->texture_map->arr[tile_id]), NULL, &dest);
				}

				if (has_entity(rain_game->map, adj_x, adj_y, z) && get_map_space(rain_game->map, adj_x, adj_y, z)->visible) {
					u32 tile_id = get_map_entity(rain_game->map, adj_x, adj_y, z)->sprite_id;

					SDL_Rect dest;
					dest.w = (i32)(((f32)((SDL_Surface *)rain_game->surface_map->arr[tile_id])->w) * rain_game->camera->scale);
					dest.h = (i32)(((f32)((SDL_Surface *)rain_game->surface_map->arr[tile_id])->h) * rain_game->camera->scale);
					dest.x = (((cam_adj_x + cam_adj_y) * 16.0) + rain_game->camera->pos_x) * rain_game->camera->scale;
					dest.y = ((((cam_adj_x - cam_adj_y) * 8.0) - (16.0 * (f32)z) + rain_game->camera->pos_y) * rain_game->camera->scale) - dest.h;

					if (game->redraw_buffer) {
						blit_surface_to_click_buffer(((SDL_Surface *)rain_game->scaled_surface_map->arr[tile_id]), &dest, game->click_map, game->screen_width, game->screen_height, threed_to_oned(adj_x, adj_y, z, rain_game->map->width, rain_game->map->height));
					}
					SDL_RenderCopy(game->renderer, ((SDL_Texture *)rain_game->texture_map->arr[tile_id]), NULL, &dest);
				}
			}
		}
	}

	SDL_Rect UI_frame;
	UI_frame.w = game->screen_width;
	UI_frame.h = game->screen_height / 7;
	UI_frame.x = game->screen_width - UI_frame.w;
	UI_frame.y = game->screen_height - UI_frame.h;
	SDL_SetRenderDrawColor(game->renderer, 40, 40, 40, 255);
	SDL_RenderFillRect(game->renderer, &UI_frame);
	if (game->redraw_buffer) {
		blit_rect_to_click_buffer(&UI_frame, game->click_map, game->screen_width, game->screen_height, rain_game->map->size);
	}

	u32 leftovers = 0;
	u32 remaining_players = 0;
	for (u32 i = 0; i < rain_game->max_players; i++) {
		if (rain_game->entity_map[i] != NULL) {
			u32 shim = game->screen_width / (rain_game->max_players * 3);
			f32 box_pos = ((f32)(i + 1) / (f32)(rain_game->max_players + 1));
			SDL_Rect player_data_box;
			player_data_box.w = game->screen_width / 8;
			player_data_box.h = UI_frame.h;
			player_data_box.x = (u32)((f32)UI_frame.w * box_pos) - player_data_box.w + shim;
			player_data_box.y = UI_frame.y + UI_frame.h - player_data_box.h;

			SDL_Rect player_healthbar_background;
			player_healthbar_background.w = player_data_box.w;
			player_healthbar_background.h = player_data_box.h / 5;
			player_healthbar_background.x = player_data_box.x;
			player_healthbar_background.y = player_data_box.y + player_data_box.h - player_healthbar_background.h;

			f32 player_health_perc = (f32)rain_game->entity_map[i]->cur_health / (f32)rain_game->entity_map[i]->max_health;

			SDL_Rect player_healthbar;
			player_healthbar.w = (u32)((f32)player_data_box.w * player_health_perc);
			player_healthbar.h = player_healthbar_background.h;
			player_healthbar.x = player_healthbar_background.x;
			player_healthbar.y = player_healthbar_background.y;

			u32 image_w = ((SDL_Surface *)rain_game->surface_map->arr[rain_game->entity_map[i]->sprite_id])->w;
			u32 image_h = ((SDL_Surface *)rain_game->surface_map->arr[rain_game->entity_map[i]->sprite_id])->h;

			f32 ratio = fmin((f32)player_data_box.w / (f32)image_w, (f32)(player_data_box.h - player_healthbar.h) / (f32)image_h);

			SDL_Rect player_image_box;
			player_image_box.w = ((f32)image_w * ratio);
			player_image_box.h = ((f32)image_h * ratio);
			player_image_box.x = player_data_box.x + (player_data_box.w / 2) - (player_image_box.w / 2);
			player_image_box.y = player_data_box.y;

			SDL_Rect player_turn_indicator;
			player_turn_indicator.w = player_data_box.w / 4;
			player_turn_indicator.h = player_data_box.w / 4;
			player_turn_indicator.x = player_data_box.x;
			player_turn_indicator.y = player_data_box.y;

			if (get_map_entity(rain_game->map, rain_game->selected_char.x, rain_game->selected_char.y, rain_game->selected_char.z) == rain_game->entity_map[i]) {
				SDL_SetRenderDrawColor(game->renderer, 110, 110, 110, 255);
			} else {
				SDL_SetRenderDrawColor(game->renderer, 60, 60, 60, 255);
			}
			SDL_RenderFillRect(game->renderer, &player_data_box);

			SDL_RenderCopy(game->renderer, ((SDL_Texture *)rain_game->texture_map->arr[rain_game->entity_map[i]->sprite_id]), NULL, &player_image_box);

			SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
			SDL_RenderFillRect(game->renderer, &player_healthbar_background);

			SDL_SetRenderDrawColor(game->renderer, 0, 90, 0, 255);
			SDL_RenderFillRect(game->renderer, &player_healthbar);

			if (rain_game->entity_map[i]->turn_points > 0) {
				SDL_SetRenderDrawColor(game->renderer, 0, 90, 0, 255);
			} else {
				SDL_SetRenderDrawColor(game->renderer, 90, 0, 0, 255);
			}

			if (game->redraw_buffer) {
				blit_rect_to_click_buffer(&player_data_box, game->click_map, game->screen_width, game->screen_height, point_to_oned(rain_game->entity_map[i]->pos, rain_game->map->width, rain_game->map->height));
			}

			SDL_RenderFillRect(game->renderer, &player_turn_indicator);

			leftovers += rain_game->entity_map[i]->turn_points;
			remaining_players++;
		}
	}

	if (leftovers == 0 && !rain_game->travelling && remaining_players > 0) {
		for (u32 i = rain_game->max_players; i < rain_game->max_enemies; i++) {
			if (rain_game->entity_map[i] != NULL) {
				rain_game->entity_map[i]->turn_points = 2;
			}
		}

		rain_game->player_turn = false;
	}
}

#endif
