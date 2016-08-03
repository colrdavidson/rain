#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_mixer.h"
#include <stdio.h>

#include "common.h"
#include "tga.h"
#include "point.h"
#include "path.h"
#include "map.h"

// Assumes a 24bit color depth for textures
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

void rescale_surfaces(SDL_Surface **surface_map, SDL_Surface **scaled_surface_map, u32 num_surfaces, f32 scale) {
	for (u32 i = 1; i < num_surfaces; i++) {
		SDL_Surface *scaled = SDL_CreateRGBSurface(0, (u32)((f32)surface_map[i]->w * scale), (u32)((f32)surface_map[i]->h * scale), surface_map[i]->format->BitsPerPixel, surface_map[i]->format->Rmask, surface_map[i]->format->Gmask, surface_map[i]->format->Bmask, surface_map[i]->format->Amask);
		SDL_BlitScaled(surface_map[i], NULL, scaled, NULL);

		SDL_FreeSurface(scaled_surface_map[i]);
		scaled_surface_map[i] = scaled;
	}
}

int main() {
	u16 original_screen_width = 640;
	u16 original_screen_height = 480;
    i32 screen_width;
    i32 screen_height;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Init(SDL_INIT_AUDIO);
	IMG_Init(IMG_INIT_PNG);
	SDL_Window *window = SDL_CreateWindow("Rain", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, original_screen_width, original_screen_height, SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_GL_GetDrawableSize(window, &screen_width, &screen_height);


	if (window == NULL) {
		printf("!window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	Mix_Init(0);
	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_Chunk *shoot_wav = Mix_LoadWAV("assets/shoot.wav");
	Mix_Chunk *step_wav = Mix_LoadWAV("assets/step.wav");
	//Mix_Music *music = Mix_LoadMUS("assets/rain.wav");

	//Mix_PlayMusic(music, -1);

	u32 click_map_size = screen_width * screen_height * sizeof(u16);
	u16 *click_map = malloc(click_map_size);

	FILE *fp = fopen("assets/house_map", "r");
	char *line = malloc(256);

	fgets(line, 256, fp);
	u32 map_width = atoi(strtok(line, " "));
	u32 map_height = atoi(strtok(NULL, " "));
	u32 map_depth = atoi(strtok(NULL, " ")) + 1;
	printf("Map Size: %dx%dx%d\n", map_width, map_height, map_depth);

	Map *map = new_map(map_width, map_height, map_depth);
	memset(click_map, map->size, click_map_size);

	fgets(line, 256, fp);
	fgets(line, 256, fp);
	u32 tile_entries = atoi(line) + 3;
	SDL_Surface *surface_map[tile_entries];
	SDL_Surface *scaled_surface_map[tile_entries];
	SDL_Texture *texture_map[tile_entries];

	u32 player_idx = tile_entries - 1;
	u32 enemy_idx = tile_entries - 2;
	for (u32 i = 1; i < tile_entries - 2; i++) {
		fgets(line, 256, fp);

		u32 idx = atoi(strtok(line, " "));
		char *img_filename = strtok(NULL, " ");
		img_filename[strlen(img_filename) - 1] = 0;

		surface_map[idx] = IMG_Load(img_filename);
		if (surface_map[idx] == NULL) {
			printf("Error opening tile %u, (%s)\n", idx, img_filename);
			return 0;
		}

		texture_map[idx] = SDL_CreateTextureFromSurface(renderer, surface_map[idx]);
	}

	surface_map[player_idx] = IMG_Load("assets/player_cylinder.png");
	texture_map[player_idx] = SDL_CreateTextureFromSurface(renderer, surface_map[player_idx]);

	surface_map[enemy_idx] = IMG_Load("assets/enemy_cylinder.png");
	texture_map[enemy_idx] = SDL_CreateTextureFromSurface(renderer, surface_map[enemy_idx]);

	f32 scale = 1.0;
	Direction direction = NORTH;

	for (u32 i = 1; i < tile_entries; i++) {
		i32 tile_width, tile_height;
		SDL_QueryTexture(texture_map[i], NULL, NULL, &tile_width, &tile_height);
		SDL_Surface *surface = SDL_CreateRGBSurface(0, (u32)tile_width, (u32)tile_height, surface_map[i]->format->BitsPerPixel, surface_map[i]->format->Rmask, surface_map[i]->format->Gmask, surface_map[i]->format->Bmask, surface_map[i]->format->Amask);
		SDL_BlitScaled(surface_map[i], NULL, surface, NULL);
		scaled_surface_map[i] = surface;
	}

	fgets(line, 256, fp);
	fgets(line, 256, fp);

	u8 max_players = 4;
	u8 max_enemies = 20;
    Entity **entity_map = malloc(sizeof(Entity *) * (max_players + max_enemies));
	u32 player_map_idx = 0;
	u32 enemy_map_idx = 0;

	for (u32 z = 0; z < map->depth - 1; z++) {
		for (u32 y = 0; y < map->height; y++) {
			u8 new_line = 1;
			for (u32 x = 0; x < map->width; x++) {
				char *bit;
				if (new_line == 1) {
					bit = strtok(line, " ");
					new_line = 0;
				} else {
					bit = strtok(NULL, " ");
				}

				if (strncmp(bit, "p", 1) == 0) {
					if (player_map_idx < max_players) {
						set_map_entity(map, x, y, z, new_entity(player_idx, 1, 5, direction));
						entity_map[player_map_idx] = get_map_entity(map, x, y, z);
						player_map_idx++;
					} else {
						puts("[MAP ERROR] Too many players!");
						return 1;
					}
				} else if (strncmp(bit, "e", 1) == 0) {
					if (enemy_map_idx < max_enemies) {
						set_map_entity(map, x, y, z, new_entity(enemy_idx, 1, 5, direction));
						entity_map[max_players + enemy_map_idx] = get_map_entity(map, x, y, z);
						enemy_map_idx++;
					} else {
						puts("[MAP ERROR] Too many enemies!");
						return 1;
					}
				} else {
					u32 tile_id = atoi(bit);
					if (tile_id) {
						set_map_tile(map, x, y, z, new_tile(tile_id), 1);
					}
				}
			}
			fgets(line, 256, fp);
		}
		fgets(line, 256, fp);
	}

	u32 entity_map_length = player_map_idx + enemy_map_idx;

    Point selected_char = new_point(0, 0, 0);

	f32 camera_pos_x = -35;
	f32 camera_pos_y = screen_height / 2;
	f32 camera_vel_x = 0.0;
	f32 camera_vel_y = 0.0;
	f32 camera_imp_x = 0.0;
	f32 camera_imp_y = 0.0;
	f32 camera_speed = 2.0;
	if (screen_width != original_screen_width) {
		camera_pos_x = -10;
		camera_pos_y = screen_height / 4;
		scale = 2.0;
		rescale_surfaces(surface_map, scaled_surface_map, tile_entries, scale);
	}

	/*
	SDL_Surface *dir_door_bmp = north_door_bmp;
	SDL_Texture *dir_door_tex = north_door_tex;
	*/

	u32 max_neighbors = 10;
	GridNode *node_map = new_nodemap(map, max_neighbors);

	Point start;
	Point goal;
	PathNode *path = NULL;
	PathNode *cur_pos = NULL;
	f32 current_time = (f32)SDL_GetTicks() / 60.0;
	f32 t = 0.0;

    u8 redraw_buffer = true;
	u8 travelling = false;

	u8 running = 1;
    while (running) {
		camera_imp_x = 0.0;
		camera_imp_y = 0.0;
		SDL_Event event;

		SDL_PumpEvents();
		const u8 *state = SDL_GetKeyboardState(NULL);
		if (state[SDL_SCANCODE_UP]) {
			camera_imp_y += camera_speed;
			redraw_buffer = true;
		}
		if (state[SDL_SCANCODE_DOWN]) {
			camera_imp_y -= camera_speed;
			redraw_buffer = true;
		}
		if (state[SDL_SCANCODE_LEFT]) {
			camera_imp_x += camera_speed;
			redraw_buffer = true;
		}
		if (state[SDL_SCANCODE_RIGHT]) {
			camera_imp_x -= camera_speed;
			redraw_buffer = true;
		}


		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN: {
					switch (event.key.keysym.sym) {
						case SDLK_e: {
							direction = cycle_right(direction);
							redraw_buffer = true;
						} break;
						case SDLK_q: {
							direction = cycle_left(direction);
							redraw_buffer = true;
						} break;
						case SDLK_1: {
							scale = 1.0;
							rescale_surfaces(surface_map, scaled_surface_map, tile_entries, scale);
							redraw_buffer = true;
						} break;
						case SDLK_2: {
							scale = 2.0;
							rescale_surfaces(surface_map, scaled_surface_map, tile_entries, scale);
							redraw_buffer = true;
						} break;
						case SDLK_3: {
							scale = 3.0;
							rescale_surfaces(surface_map, scaled_surface_map, tile_entries, scale);
							redraw_buffer = true;
						} break;
					}
				} break;
				case SDL_MOUSEBUTTONDOWN: {
					i32 mouse_x, mouse_y;
					u32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

					mouse_x = ((f32)screen_width / (f32)original_screen_width) * mouse_x;
					mouse_y = ((f32)screen_height / (f32)original_screen_height) * mouse_y;

					Point p = oned_to_threed(click_map[twod_to_oned(mouse_x, mouse_y, screen_width)], map->width, map->height);

					if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
						Point hover_p = new_point(p.x, p.y, p.z + 1);
						if (has_entity(map, p.x, p.y, p.z) && get_map_entity(map, p.x, p.y, p.z)->sprite_id == player_idx && !travelling) {
							selected_char = p;
						} else if (can_move(map, selected_char, hover_p) && get_map_entity(map, selected_char.x, selected_char.y, selected_char.z)->sprite_id == player_idx) {
							if (path == NULL && cur_pos == NULL) {
								start = selected_char;
								goal = p;
								goal.z += 1;
								path = find_path(start, goal, node_map, map->width, map->height, map->depth, max_neighbors);
								if (path != NULL) {
									get_map_entity(map, selected_char.x, selected_char.y, selected_char.z)->turn_points--;
									cur_pos = path->next;
									if (cur_pos == NULL) {
										puts("cur_pos == NULL");
									}
									travelling = true;
								}
							}
						}
					} else if (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
						if (has_entity(map, p.x, p.y, p.z) && get_map_entity(map, p.x, p.y, p.z)->sprite_id == enemy_idx && has_entity(map, selected_char.x, selected_char.y, selected_char.z) && get_map_entity(map, selected_char.x, selected_char.y, selected_char.z)->sprite_id == player_idx) {
							u32 turns = get_map_entity(map, selected_char.x, selected_char.y, selected_char.z)->turn_points;
							if (turns == 1) {
								get_map_entity(map, selected_char.x, selected_char.y, selected_char.z)->turn_points = 0;
							} else if (turns > 1) {
								get_map_entity(map, selected_char.x, selected_char.y, selected_char.z)->turn_points -= 2;
							} else {
								continue;
							}

							u32 health = get_map_entity(map, selected_char.x, selected_char.y, selected_char.z)->cur_health;
							if (health != 0) {
								Mix_PlayChannel(-1, shoot_wav, 0);
								get_map_entity(map, selected_char.x, selected_char.y, selected_char.z)->cur_health--;

								Entity *e = get_map_entity(map, p.x, p.y, p.z);
								for (u32 i = 0; i < max_enemies; i++) {
									if (e == entity_map[i]) {
										entity_map[i] = NULL;
										break;
									}
								}

								set_map_entity(map, p.x, p.y, p.z, NULL);
								redraw_buffer = true;
							}
						}
					}

				} break;
				case SDL_QUIT: {
					running = false;
				} break;
			}
		}


		f32 new_time = (f32)SDL_GetTicks() / 60.0;
		f32 dt = new_time - current_time;
		current_time = new_time;

		f32 friction = -0.2;
		f32 x_acc = friction * camera_vel_x + camera_imp_x;
		f32 y_acc = friction * camera_vel_y + camera_imp_y;
		camera_pos_x = (0.5 * x_acc * dt * dt) + camera_vel_x * dt + camera_pos_x;
		camera_pos_y = (0.5 * y_acc * dt * dt) + camera_vel_y * dt + camera_pos_y;
		camera_vel_x = (x_acc * dt) + camera_vel_x;
		camera_vel_y = (y_acc * dt) + camera_vel_y;

		if (cur_pos != NULL) {
			if (t > 3.0) {
				Mix_PlayChannel(-1, step_wav, 0);
				move_entity(map, selected_char, cur_pos->p);
				selected_char = cur_pos->p;
				cur_pos = cur_pos->next;
				t = 0.0;
				redraw_buffer = true;
			}
		} else {
			if (path != NULL) {
				free_path(path);
				path = NULL;
				travelling = false;
			}
		}

		if (redraw_buffer) {
			memset(click_map, map->size, click_map_size);
		}

		u8 redraw_lights = false;
		for (u32 i = 0; i < entity_map_length; i++) {
			if (entity_map[i] != NULL && entity_map[i]->moved == true) {
				redraw_lights = true;
			}
		}

		if (redraw_lights) {
			for (u32 i = 0; i < map->size; i++) {
				map->spaces[i]->visible = false;
			}

			for (u32 i = 0; i < max_players; i++) {
				if (entity_map[i] != NULL) {
					update_visible(map, entity_map[i], true);
					entity_map[i]->moved = false;
				}
			}

			for (u32 i = max_players; i < entity_map_length; i++) {
				if (entity_map[i] != NULL) {
					update_visible(map, entity_map[i], false);
					entity_map[i]->moved = false;
				}
			}
		}

		t += dt;

		/*
		 * The directional tile system needs to be managed in the dynamic loader
		 */

		SDL_RenderClear(renderer);

		for (u32 z = 0; z < map->depth; z++) {
			for (u32 x = 0; x < map->width; x++) {
				for (u32 y = 0; y < map->height; y++) {

					f32 adj_y;
					f32 adj_x;

					f32 cam_adj_x;
					f32 cam_adj_y;

					switch (direction) {
						case NORTH: {
							adj_y = y;
							adj_x = x;

							cam_adj_x = x;
							cam_adj_y = map->height - y;

							/*
							dir_door_bmp = north_door_bmp;
							dir_door_tex = north_door_tex;
							*/
						} break;
						case EAST: {
							adj_y = map->height - y - 1;
							adj_x = x;

							cam_adj_x = y;
							cam_adj_y = map->width - x;
						} break;
						case SOUTH: {
							adj_y = map->height - y - 1;
							adj_x = map->width - x - 1;

							cam_adj_x = x;
							cam_adj_y = map->height - y;
						} break;
						case WEST: {
							adj_x = map->width - x - 1;
							adj_y = y;

							cam_adj_x = y;
							cam_adj_y = map->width - x;

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
							cam_adj_y = map->height - y;

							/*
							dir_door_bmp = north_door_bmp;
							dir_door_tex = north_door_tex;
							*/
						} break;
					}

					if (has_tile(map, adj_x, adj_y, z)) {
                        u32 tile_id = get_map_tile(map, adj_x, adj_y, z)->sprite_id;

						SDL_Rect dest;
						dest.w = (i32)((f32)(surface_map[tile_id]->w) * scale);
						dest.h = (i32)((f32)(surface_map[tile_id]->h) * scale);
						dest.x = (((cam_adj_x + cam_adj_y) * 16.0) + camera_pos_x) * scale;
						dest.y = ((((cam_adj_x - cam_adj_y) * 8.0) - (16.0 * (f32)z) + camera_pos_y) * scale) - dest.h;


						if (get_map_space(map, adj_x, adj_y, z)->visible) {
							SDL_SetTextureColorMod(texture_map[tile_id], 255, 255, 255);

							if (redraw_buffer) {
								blit_surface_to_click_buffer(scaled_surface_map[tile_id], &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map->width, map->height));
							}
						} else {
							SDL_SetTextureColorMod(texture_map[tile_id], 100, 100, 100);
						}
						SDL_RenderCopy(renderer, texture_map[tile_id], NULL, &dest);
					}

					if (has_entity(map, adj_x, adj_y, z) && get_map_space(map, adj_x, adj_y, z)->visible) {
                        u32 tile_id = get_map_entity(map, adj_x, adj_y, z)->sprite_id;

						SDL_Rect dest;
						dest.w = (i32)((f32)(surface_map[tile_id]->w) * scale);
						dest.h = (i32)((f32)(surface_map[tile_id]->h) * scale);
						dest.x = (((cam_adj_x + cam_adj_y) * 16.0) + camera_pos_x) * scale;
						dest.y = ((((cam_adj_x - cam_adj_y) * 8.0) - (16.0 * (f32)z) + camera_pos_y) * scale) - dest.h;

						if (redraw_buffer) {
							blit_surface_to_click_buffer(scaled_surface_map[tile_id], &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map->width, map->height));
						}
						SDL_RenderCopy(renderer, texture_map[tile_id], NULL, &dest);
					}
				}
			}
		}

		SDL_Rect UI_frame;
		UI_frame.w = screen_width;
		UI_frame.h = screen_height / 7;
		UI_frame.x = screen_width - UI_frame.w;
		UI_frame.y = screen_height - UI_frame.h;
		SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
		SDL_RenderFillRect(renderer, &UI_frame);

		u32 leftovers = 0;
		for (u32 i = 0; i < max_players; i++) {
			u32 shim = screen_width / (max_players * 3);
            f32 box_pos = ((f32)(i + 1) / (f32)(max_players + 1));
			SDL_Rect player_data_box;
			player_data_box.w = screen_width / 6;
			player_data_box.h = UI_frame.h;
			player_data_box.x = (u32)((f32)UI_frame.w * box_pos) - player_data_box.w + shim;
			player_data_box.y = UI_frame.y + UI_frame.h - player_data_box.h;

			SDL_Rect player_healthbar_background;
			player_healthbar_background.w = player_data_box.w;
			player_healthbar_background.h = player_data_box.h / 5;
			player_healthbar_background.x = player_data_box.x;
			player_healthbar_background.y = player_data_box.y + player_data_box.h - player_healthbar_background.h;

			f32 player_health_perc = (f32)entity_map[i]->cur_health / (f32)entity_map[i]->max_health;

			SDL_Rect player_healthbar;
			player_healthbar.w = (u32)((f32)player_data_box.w * player_health_perc);
			player_healthbar.h = player_healthbar_background.h;
			player_healthbar.x = player_healthbar_background.x;
			player_healthbar.y = player_healthbar_background.y;

			SDL_Rect player_image_box;
			player_image_box.w = player_data_box.w;
			player_image_box.h = player_data_box.h - player_healthbar.h;
			player_image_box.x = player_data_box.x;
			player_image_box.y = player_data_box.y;

			SDL_Rect player_turn_indicator;
			player_turn_indicator.w = player_data_box.w / 4;
			player_turn_indicator.h = player_data_box.h / 4;
			player_turn_indicator.x = player_data_box.x;
			player_turn_indicator.y = player_data_box.y;

			if (get_map_entity(map, selected_char.x, selected_char.y, selected_char.z) == entity_map[i]) {
				SDL_SetRenderDrawColor(renderer, 110, 110, 110, 255);
			} else {
				SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
			}
			SDL_RenderFillRect(renderer, &player_data_box);

			SDL_RenderCopy(renderer, texture_map[entity_map[i]->sprite_id], NULL, &player_image_box);

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderFillRect(renderer, &player_healthbar_background);

			SDL_SetRenderDrawColor(renderer, 0, 90, 0, 255);
			SDL_RenderFillRect(renderer, &player_healthbar);

			if (entity_map[i]->turn_points > 0) {
				SDL_SetRenderDrawColor(renderer, 0, 90, 0, 255);
			} else {
				SDL_SetRenderDrawColor(renderer, 90, 0, 0, 255);
			}

			SDL_RenderFillRect(renderer, &player_turn_indicator);

			leftovers += entity_map[i]->turn_points;
		}

		if (leftovers == 0 && !travelling) {
			puts("turn over");
			for (u32 i = 0; i < entity_map_length; i++) {
				if (entity_map[i] != NULL) {
					entity_map[i]->turn_points = 2;
				}
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		redraw_buffer = false;
		SDL_RenderPresent(renderer);
	}


	Image img;
    img.width = screen_width;
    img.height = screen_height;
    img.bytes_per_pixel = 2;
	img.data = (Color *)click_map;
	write_tga("test.tga", &img);

	IMG_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
    return 0;
}
