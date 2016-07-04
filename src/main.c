#include "SDL2/SDL.h"
#include <stdio.h>

#include "common.h"
#include "tga.h"
#include "point.h"
#include "path.h"

// Assumes a 24bit color depth for textures
void blit_surface_to_click_buffer(SDL_Surface *surface, SDL_Rect *screen_rel_rect, u32 *click_map, u32 screen_width, u32 screen_height, u32 tile_num) {
	u32 pchunk_ptr = 0;

	if ((screen_rel_rect->x >= (i32)screen_width && (screen_rel_rect->x + screen_rel_rect->w) >= (i32)screen_width) || (screen_rel_rect->y >= (i32)screen_height && (screen_rel_rect->y + screen_rel_rect->h) >= (i32)screen_height)) {
		return;
	}

	for (i32 i = 0; i < surface->w * surface->h; i++) {
		u32 pixel = ((u8 *)surface->pixels)[pchunk_ptr + 2] << 16 | ((u8 *)surface->pixels)[pchunk_ptr + 1] << 8 | ((u8 *)surface->pixels)[pchunk_ptr];
		if (pixel != 0) {
			Point pix_pos = oned_to_twod(i, surface->w);
			if ((screen_rel_rect->x + pix_pos.x < screen_width) && (screen_rel_rect->y + pix_pos.y < screen_height)) {
				u32 click_idx = twod_to_oned(screen_rel_rect->x + pix_pos.x, screen_rel_rect->y + pix_pos.y, screen_width);
				if (click_idx < screen_width * screen_height) {
					click_map[click_idx] = tile_num;
				}
			}
		}
		pchunk_ptr += 3;
	}
}

void rescale_surfaces(SDL_Surface **surface_map, SDL_Surface **scaled_surface_map, u32 num_surfaces, i32 tile_width, i32 tile_height, f32 scale) {
	for (u32 i = 1; i < num_surfaces; i++) {
		SDL_Surface *scaled = SDL_CreateRGBSurface(0, (u32)((f32)tile_width * scale), (u32)((f32)tile_height * scale), surface_map[i]->format->BitsPerPixel, surface_map[i]->format->Rmask, surface_map[i]->format->Gmask, surface_map[i]->format->Bmask, surface_map[i]->format->Amask);
		SDL_BlitScaled(surface_map[i], NULL, scaled, NULL);
		free(scaled_surface_map[i]);
		scaled_surface_map[i] = scaled;
	}
}

int main() {
	u16 original_screen_width = 640;
	u16 original_screen_height = 480;
    i32 screen_width;
    i32 screen_height;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow("Rain", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, original_screen_width, original_screen_height, SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_GL_GetDrawableSize(window, &screen_width, &screen_height);


	if (window == NULL) {
		printf("!window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

	u32 *click_map = malloc(screen_width * screen_height * sizeof(u32));
	memset(click_map, 0, screen_width * screen_height * sizeof(u32));

	FILE *fp = fopen("assets/house_map", "r");
	char *line = malloc(256);

	fgets(line, 256, fp);
	u32 map_width = atoi(strtok(line, " "));
	u32 map_height = atoi(strtok(NULL, " "));
	u32 map_depth = atoi(strtok(NULL, " ")) + 1;
	printf("Map Size: %dx%dx%d\n", map_width, map_height, map_depth);

	u8 map[map_width * map_depth * map_height];
	memset(map, 0, sizeof(map));

	fgets(line, 256, fp);
	fgets(line, 256, fp);
	u32 tile_entries = atoi(line) + 2;
	SDL_Surface *surface_map[tile_entries];
	SDL_Surface *scaled_surface_map[tile_entries];
	SDL_Texture *texture_map[tile_entries];

	i32 player_idx = tile_entries - 1;
	for (u32 i = 1; i < tile_entries - 1; i++) {
		fgets(line, 256, fp);

		u32 idx = atoi(strtok(line, " "));
		char *bmp_filename = strtok(NULL, " ");
		bmp_filename[strlen(bmp_filename) - 1] = 0;

		surface_map[idx] = SDL_LoadBMP(bmp_filename);
		if (surface_map[idx] == NULL) {
			printf("Error opening tile %u, (%s)\n", idx, bmp_filename);
			return 0;
		}

		SDL_SetColorKey(surface_map[idx], SDL_TRUE, SDL_MapRGB(surface_map[idx]->format, 0, 0, 0));
		texture_map[idx] = SDL_CreateTextureFromSurface(renderer, surface_map[idx]);
	}

	surface_map[player_idx] = SDL_LoadBMP("assets/cylinder.bmp");
	SDL_SetColorKey(surface_map[player_idx], SDL_TRUE, SDL_MapRGB(surface_map[player_idx]->format, 0, 0, 0));
	texture_map[player_idx] = SDL_CreateTextureFromSurface(renderer, surface_map[player_idx]);

	i32 tile_width, tile_height;
	SDL_QueryTexture(texture_map[1], NULL, NULL, &tile_width, &tile_height);
	f32 scale = 1.0;

	for (u32 i = 1; i < tile_entries; i++) {
		SDL_Surface *surface = SDL_CreateRGBSurface(0, (u32)tile_width, (u32)tile_height, surface_map[i]->format->BitsPerPixel, surface_map[i]->format->Rmask, surface_map[i]->format->Gmask, surface_map[i]->format->Bmask, surface_map[i]->format->Amask);
		SDL_BlitScaled(surface_map[i], NULL, surface, NULL);
		scaled_surface_map[i] = surface;
	}

	fgets(line, 256, fp);
	fgets(line, 256, fp);

	for (u8 z = 0; z < map_depth - 1; z++) {
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
				map[threed_to_oned(x, y, z, map_width, map_height)] = tile_id;
			}
			fgets(line, 256, fp);
		}
		fgets(line, 256, fp);
	}

	Point player = new_point(0, 0, 1);
	map[threed_to_oned(player.x, player.y, player.z, map_width, map_height)] = player_idx;


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
		rescale_surfaces(surface_map, scaled_surface_map, tile_entries, tile_width, tile_height, scale);
	}

	Direction direction = NORTH;
	/*
	SDL_Surface *dir_door_bmp = north_door_bmp;
	SDL_Texture *dir_door_tex = north_door_tex;
	*/

	u32 max_neighbors = 10;
	GridNode *node_map = malloc(sizeof(GridNode) * map_width * map_height * map_depth);
	for (u32 x = 0; x < map_width; x++) {
		for (u32 y = 0; y < map_height; y++) {
			for (u32 z = 0; z < map_depth; z++) {
				GridNode tmp;
				tmp.tile_pos = threed_to_oned(x, y, z, map_width, map_height);
				tmp.tile_type = map[tmp.tile_pos];
				tmp.neighbors = malloc(sizeof(GridNode) * max_neighbors);

				node_map[threed_to_oned(x, y, z, map_width, map_height)] = tmp;
			}
		}
	}

	for (u32 y = 0; y < map_height; y++) {
    	for (u32 x = 0; x < map_width; x++) {
    		for (u32 z = 0; z < map_depth; z++) {
				node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[SOUTH] = NULL;
				node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[EAST] = NULL;
				node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[WEST] = NULL;
				node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[NORTH] = NULL;
				node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[UP] = NULL;
				node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[DOWN] = NULL;
				node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[NORTHEAST] = NULL;
				node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[NORTHWEST] = NULL;
				node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[SOUTHEAST] = NULL;
				node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[SOUTHWEST] = NULL;

				if (y < map_height - 1) {
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[SOUTH] = &node_map[threed_to_oned(x, y + 1, z, map_width, map_height)];
				}
				if (x < map_width - 1) {
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[EAST] = &node_map[threed_to_oned(x + 1, y, z, map_width, map_height)];
				}
				if (x > 0) {
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[WEST] = &node_map[threed_to_oned(x - 1, y, z, map_width, map_height)];
				}
				if (y > 0) {
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[NORTH] = &node_map[threed_to_oned(x, y - 1, z, map_width, map_height)];
				}

				if (x > 0 && y > 0) {
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[NORTHWEST] = &node_map[threed_to_oned(x - 1, y - 1, z, map_width, map_height)];
				}

				if (x < map_width - 1 && y > 0) {
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[NORTHEAST] = &node_map[threed_to_oned(x + 1, y - 1, z, map_width, map_height)];
				}

				if (x < map_width - 1 && y < map_height - 1) {
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[SOUTHWEST] = &node_map[threed_to_oned(x + 1, y + 1, z, map_width, map_height)];
				}

				if (x > 0 && y < map_height - 1) {
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[SOUTHEAST] = &node_map[threed_to_oned(x - 1, y + 1, z, map_width, map_height)];
				}

				if (z > 0 && map[threed_to_oned(x, y, z - 1, map_width, map_height)] == 0) {
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[SOUTH] = NULL;
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[EAST] = NULL;
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[WEST] = NULL;
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[NORTH] = NULL;
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[UP] = NULL;
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[DOWN] = NULL;
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[NORTHEAST] = NULL;
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[NORTHWEST] = NULL;
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[SOUTHEAST] = NULL;
					node_map[threed_to_oned(x, y, z, map_width, map_height)].neighbors[SOUTHWEST] = NULL;
				}


				/*
				 * Fixed ID's are no longer useful in the new dynamic loader. To re-enable ladders,
				 * map files need to be able to denote block descriptors
				 *
				if (z < map_depth - 2 && map[threed_to_oned(x, y, z + 1, map_width, map_height)] == 9) {
					node_map[threed_to_oned(x, y - 1, z, map_width, map_height)].neighbors[UP] = &node_map[threed_to_oned(x, y - 1, z + 1, map_width, map_height)];
					node_map[threed_to_oned(x, y - 1, z + 1, map_width, map_height)].neighbors[UP] = &node_map[threed_to_oned(x, y - 1, z + 2, map_width, map_height)];

					node_map[threed_to_oned(x, y - 1, z + 1, map_width, map_height)].neighbors[DOWN] = &node_map[threed_to_oned(x, y - 1, z, map_width, map_height)];
					node_map[threed_to_oned(x, y - 1, z + 2, map_width, map_height)].neighbors[DOWN] = &node_map[threed_to_oned(x, y - 1, z + 1, map_width, map_height)];
				}
				*/
			}
		}
	}

	Point start;
	Point goal;
	PathNode *path = NULL;
	PathNode *cur_pos = NULL;
	f32 current_time = (f32)SDL_GetTicks() / 60.0;
	f32 t = 0.0;

    u8 redraw_buffer = 1;


	u8 running = 1;
    while (running) {
		camera_imp_x = 0.0;
		camera_imp_y = 0.0;
		SDL_Event event;

		SDL_PumpEvents();
		const u8 *state = SDL_GetKeyboardState(NULL);
		u8 triggered = 0;
		if (state[SDL_SCANCODE_UP]) {
			camera_imp_y += camera_speed;
			triggered = 1;
		}
		if (state[SDL_SCANCODE_DOWN]) {
			camera_imp_y -= camera_speed;
			triggered = 1;
		}
		if (state[SDL_SCANCODE_LEFT]) {
			camera_imp_x += camera_speed;
			triggered = 1;
		}
		if (state[SDL_SCANCODE_RIGHT]) {
			camera_imp_x -= camera_speed;
			triggered = 1;
		}


		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN: {
					switch (event.key.keysym.sym) {
						case SDLK_e: {
							direction = cycle_right(direction);
							triggered = 1;
						} break;
						case SDLK_q: {
							direction = cycle_left(direction);
							triggered = 1;
						} break;
						case SDLK_1: {
							scale = 1.0;
							rescale_surfaces(surface_map, scaled_surface_map, tile_entries, tile_width, tile_height, scale);
							triggered = 1;
						} break;
						case SDLK_2: {
							scale = 2.0;
							rescale_surfaces(surface_map, scaled_surface_map, tile_entries, tile_width, tile_height, scale);
							triggered = 1;
						} break;
						case SDLK_3: {
							scale = 3.0;
							rescale_surfaces(surface_map, scaled_surface_map, tile_entries, tile_width, tile_height, scale);
							triggered = 1;
						} break;
					}
				} break;
				case SDL_MOUSEBUTTONDOWN: {
					i32 mouse_x, mouse_y;
					u32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

					mouse_x = ((f32)screen_width / (f32)original_screen_width) * mouse_x;
					mouse_y = ((f32)screen_height / (f32)original_screen_height) * mouse_y;

					Point p = oned_to_threed(click_map[twod_to_oned(mouse_x, mouse_y, screen_width)], map_width, map_height);

					if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
						if (map[threed_to_oned(p.x, p.y, p.z + 1, map_width, map_height)] == 0) {
							start = player;
							goal = p;
							goal.z += 1;

							if (path == NULL && cur_pos == NULL) {
								path = find_path(start, goal, node_map, map_width, map_height, map_depth, max_neighbors, player_idx);
								cur_pos = path;
							}
						}
					}

				} break;
				case SDL_QUIT: {
					running = 0;
				} break;
			}
		}

		if (triggered) {
			memset(click_map, 0, screen_width * screen_height * sizeof(u32));
			redraw_buffer = 1;
			triggered = 0;
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
				map[threed_to_oned(player.x, player.y, player.z, map_width, map_height)] = 0;
				player = cur_pos->p;
				map[threed_to_oned(player.x, player.y, player.z, map_width, map_height)] = player_idx;
				cur_pos = cur_pos->next;
				t = 0.0;
				memset(click_map, 0, screen_width * screen_height * sizeof(u32));
				redraw_buffer = 1;
			}
		} else {
			if (path != NULL) {
				free(path);
				path = NULL;
			}
		}

		t += dt;

		/*
		 * The directional tile system needs to be managed in the dynamic loader
		 */

		SDL_RenderClear(renderer);


		SDL_Rect dest;
		dest.w = (i32)(tile_width * scale);
		dest.h = (i32)(tile_height * scale);

		for (u32 z = 0; z < map_depth; z++) {
			for (u32 x = 0; x < map_width; x++) {
				for (u32 y = 0; y < map_height; y++) {

					f32 adj_y;
					f32 adj_x;

					f32 cam_adj_x;
					f32 cam_adj_y;

					switch (direction) {
						case NORTH: {
							adj_y = y;
							adj_x = x;

							cam_adj_x = x;
							cam_adj_y = map_height - y;

							/*
							dir_door_bmp = north_door_bmp;
							dir_door_tex = north_door_tex;
							*/
						} break;
						case EAST: {
							adj_y = map_height - y - 1;
							adj_x = x;

							cam_adj_x = y;
							cam_adj_y = map_width - x;
						} break;
						case SOUTH: {
							adj_y = map_height - y - 1;
							adj_x = map_width - x - 1;

							cam_adj_x = x;
							cam_adj_y = map_height - y;
						} break;
						case WEST: {
							adj_x = map_width - x - 1;
							adj_y = y;

							cam_adj_x = y;
							cam_adj_y = map_width - x;

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
							cam_adj_y = map_height - y;

							/*
							dir_door_bmp = north_door_bmp;
							dir_door_tex = north_door_tex;
							*/
						} break;
					}

					dest.x = (((cam_adj_x + cam_adj_y) * 16.0) + camera_pos_x) * scale;
					dest.y = (((cam_adj_x - cam_adj_y) * 8.0) - (16.0 * (f32)z) + camera_pos_y) * scale;

					u8 tile_id = map[threed_to_oned(adj_x, adj_y, z, map_width, map_height)];

					if (tile_id != 0) {
						if (redraw_buffer) {
							blit_surface_to_click_buffer(scaled_surface_map[tile_id], &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
						}
						SDL_RenderCopy(renderer, texture_map[tile_id], NULL, &dest);
					}
				}
			}
		}

		redraw_buffer = 0;
		SDL_RenderPresent(renderer);
	}

	Image img;
    img.width = screen_width;
    img.height = screen_height;
    img.bytes_per_pixel = 4;
	img.data = (Color *)click_map;
	write_tga("test.tga", &img);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
    return 0;
}
