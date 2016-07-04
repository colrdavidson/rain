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

	SDL_Surface *scaled = SDL_CreateRGBSurface(0, screen_rel_rect->w, screen_rel_rect->h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);

	SDL_BlitScaled(surface, NULL, scaled, NULL);

	for (i32 i = 0; i < scaled->w * scaled->h; i++) {
		u32 pixel = ((u8 *)scaled->pixels)[pchunk_ptr + 2] << 16 | ((u8 *)scaled->pixels)[pchunk_ptr + 1] << 8 | ((u8 *)scaled->pixels)[pchunk_ptr];
		if (pixel != 0) {
			Point pix_pos = oned_to_twod(i, scaled->w);
			if ((screen_rel_rect->x + pix_pos.x < screen_width) && (screen_rel_rect->y + pix_pos.y < screen_height)) {
				u32 click_idx = twod_to_oned(screen_rel_rect->x + pix_pos.x, screen_rel_rect->y + pix_pos.y, screen_width);
				if (click_idx < screen_width * screen_height) {
					click_map[click_idx] = tile_num;
				}
			}
		}
		pchunk_ptr += 3;
	}

	SDL_FreeSurface(scaled);
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

	SDL_Surface *wall_bmp = SDL_LoadBMP("assets/wall.bmp");
	SDL_Surface *brick_bmp = SDL_LoadBMP("assets/brick.bmp");
	SDL_Surface *grass_bmp = SDL_LoadBMP("assets/grass.bmp");
	SDL_Surface *wood_wall_bmp = SDL_LoadBMP("assets/wood_wall.bmp");
	SDL_Surface *wood_ladder_wall_bmp = SDL_LoadBMP("assets/wood_ladder_wall.bmp");
	SDL_Surface *north_door_bmp = SDL_LoadBMP("assets/north_door.bmp");
	SDL_Surface *west_door_bmp = SDL_LoadBMP("assets/west_door.bmp");
	SDL_Surface *roof_bmp = SDL_LoadBMP("assets/roof.bmp");
	SDL_Surface *cylinder_bmp = SDL_LoadBMP("assets/cylinder.bmp");
	SDL_Surface *white_brick_bmp = SDL_LoadBMP("assets/white_brick.bmp");

	SDL_SetColorKey(wall_bmp, SDL_TRUE, SDL_MapRGB(wall_bmp->format, 0, 0, 0));
	SDL_SetColorKey(brick_bmp, SDL_TRUE, SDL_MapRGB(brick_bmp->format, 0, 0, 0));
	SDL_SetColorKey(grass_bmp, SDL_TRUE, SDL_MapRGB(grass_bmp->format, 0, 0, 0));
	SDL_SetColorKey(wood_wall_bmp, SDL_TRUE, SDL_MapRGB(wood_wall_bmp->format, 0, 0, 0));
	SDL_SetColorKey(wood_ladder_wall_bmp, SDL_TRUE, SDL_MapRGB(wood_ladder_wall_bmp->format, 0, 0, 0));
	SDL_SetColorKey(north_door_bmp, SDL_TRUE, SDL_MapRGB(north_door_bmp->format, 0, 0, 0));
	SDL_SetColorKey(west_door_bmp, SDL_TRUE, SDL_MapRGB(west_door_bmp->format, 0, 0, 0));
	SDL_SetColorKey(roof_bmp, SDL_TRUE, SDL_MapRGB(roof_bmp->format, 0, 0, 0));
	SDL_SetColorKey(cylinder_bmp, SDL_TRUE, SDL_MapRGB(cylinder_bmp->format, 0, 0, 0));
	SDL_SetColorKey(white_brick_bmp, SDL_TRUE, SDL_MapRGB(white_brick_bmp->format, 0, 0, 0));

	SDL_Texture *wall_tex = SDL_CreateTextureFromSurface(renderer, wall_bmp);
	SDL_Texture *brick_tex = SDL_CreateTextureFromSurface(renderer, brick_bmp);
	SDL_Texture *white_brick_tex = SDL_CreateTextureFromSurface(renderer, white_brick_bmp);
	SDL_Texture *grass_tex = SDL_CreateTextureFromSurface(renderer, grass_bmp);
	SDL_Texture *wood_wall_tex = SDL_CreateTextureFromSurface(renderer, wood_wall_bmp);
	SDL_Texture *wood_ladder_wall_tex = SDL_CreateTextureFromSurface(renderer, wood_ladder_wall_bmp);
	SDL_Texture *north_door_tex = SDL_CreateTextureFromSurface(renderer, north_door_bmp);
	SDL_Texture *west_door_tex = SDL_CreateTextureFromSurface(renderer, west_door_bmp);
	SDL_Texture *roof_tex = SDL_CreateTextureFromSurface(renderer, roof_bmp);
	SDL_Texture *cylinder_tex = SDL_CreateTextureFromSurface(renderer, cylinder_bmp);

	FILE *fp = fopen("assets/map", "r");
	char *line = malloc(256);

	fgets(line, 256, fp);
	u32 map_width = atoi(strtok(line, " "));
	u32 map_height = atoi(strtok(NULL, " "));
	u32 map_depth = atoi(strtok(NULL, " ")) + 1;
	printf("Map Size: %dx%dx%d\n", map_width, map_height, map_depth);

	u8 map[map_width * map_depth * map_height];
	memset(map, 0, sizeof(map));

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


	Point player = new_point(10, 10, 4);
	map[threed_to_oned(player.x, player.y, player.z, map_width, map_height)] = 7;

	i32 camera_x = -35;
	i32 camera_y = screen_height / 2;
	u32 scale = 1;
	if (screen_width != original_screen_width) {
		camera_x = -10;
		camera_y = screen_height / 4;
		scale = 2;
	}

	Direction direction = NORTH;
	SDL_Surface *dir_door_bmp = north_door_bmp;
	SDL_Texture *dir_door_tex = north_door_tex;


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

				if (z < map_depth - 2 && map[threed_to_oned(x, y, z + 1, map_width, map_height)] == 9) {
					node_map[threed_to_oned(x, y - 1, z, map_width, map_height)].neighbors[UP] = &node_map[threed_to_oned(x, y - 1, z + 1, map_width, map_height)];
					node_map[threed_to_oned(x, y - 1, z + 1, map_width, map_height)].neighbors[UP] = &node_map[threed_to_oned(x, y - 1, z + 2, map_width, map_height)];

					node_map[threed_to_oned(x, y - 1, z + 1, map_width, map_height)].neighbors[DOWN] = &node_map[threed_to_oned(x, y - 1, z, map_width, map_height)];
					node_map[threed_to_oned(x, y - 1, z + 2, map_width, map_height)].neighbors[DOWN] = &node_map[threed_to_oned(x, y - 1, z + 1, map_width, map_height)];
				}
			}
		}
	}

	Point start;
	Point goal;
	PathNode *path = NULL;
	PathNode *cur_pos = NULL;
	float current_time = (float)SDL_GetTicks() / 60.0;
	float t = 0.0;

    u8 redraw_buffer = 1;

	u8 running = 1;
    while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN: {
					switch (event.key.keysym.sym) {
						case SDLK_e: {
							direction = cycle_right(direction);
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
							redraw_buffer = 1;
						} break;
						case SDLK_q: {
							direction = cycle_left(direction);
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
							redraw_buffer = 1;
						} break;
						case SDLK_1: {
							scale = 1;
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
							redraw_buffer = 1;
						} break;
						case SDLK_2: {
							scale = 2;
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
							redraw_buffer = 1;
						} break;
						case SDLK_3: {
							scale = 3;
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
							redraw_buffer = 1;
						} break;
						case SDLK_UP: {
							camera_y -= 10;
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
							redraw_buffer = 1;
						} break;
						case SDLK_DOWN: {
							camera_y += 10;
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
							redraw_buffer = 1;
						} break;
						case SDLK_LEFT: {
							camera_x -= 10;
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
							redraw_buffer = 1;
						} break;
						case SDLK_RIGHT: {
							camera_x += 10;
							memset(click_map, 0, screen_width * screen_height * sizeof(u32));
							redraw_buffer = 1;
						} break;
					}
				} break;
				case SDL_MOUSEBUTTONDOWN: {
					i32 mouse_x, mouse_y;
					u32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

					mouse_x = ((float)screen_width / (float)original_screen_width) * mouse_x;
					mouse_y = ((float)screen_height / (float)original_screen_height) * mouse_y;

					Point p = oned_to_threed(click_map[twod_to_oned(mouse_x, mouse_y, screen_width)], map_width, map_height);

					if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
						if (map[threed_to_oned(p.x, p.y, p.z + 1, map_width, map_height)] == 0) {
							start = player;
							goal = p;
							goal.z += 1;

							if (path == NULL && cur_pos == NULL) {
								path = find_path(start, goal, node_map, map_width, map_height, map_depth, max_neighbors);
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

		float new_time = (float)SDL_GetTicks() / 60.0;
		float frame_time = new_time - current_time;
		current_time = new_time;

		if (cur_pos != NULL) {
			if (t > 3.0) {
				map[threed_to_oned(player.x, player.y, player.z, map_width, map_height)] = 0;
				player = cur_pos->p;
				map[threed_to_oned(player.x, player.y, player.z, map_width, map_height)] = 7;
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

		t += frame_time;

		if (redraw_buffer) {
			SDL_RenderClear(renderer);

			i32 box_width, box_height;
			SDL_QueryTexture(wall_tex, NULL, NULL, &box_width, &box_height);

			SDL_Rect dest;
			dest.w = box_width * scale;
			dest.h = box_height * scale;

			for (u32 z = 0; z < map_depth; z++) {
				for (u32 x = 0; x < map_width; x++) {
					for (u32 y = 0; y < map_height; y++) {

						u32 adj_y;
						u32 adj_x;

						u32 cam_adj_x;
						u32 cam_adj_y;

						switch (direction) {
							case NORTH: {
								adj_y = y;
								adj_x = x;

								cam_adj_x = x;
								cam_adj_y = map_height - y;

								dir_door_bmp = north_door_bmp;
								dir_door_tex = north_door_tex;
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

								dir_door_bmp = west_door_bmp;
								dir_door_tex = west_door_tex;
							} break;
							default: {
								puts("direction wtf?");
								adj_y = y;
								adj_x = x;

								cam_adj_x = x;
								cam_adj_y = map_height - y;

								dir_door_bmp = north_door_bmp;
								dir_door_tex = north_door_tex;
							} break;
						}

						dest.x = (((cam_adj_x + cam_adj_y) * 16) + camera_x) * scale;
						dest.y = (((cam_adj_x - cam_adj_y) * 8) - (16 * z) + camera_y) * scale;

						u8 tile_id = map[threed_to_oned(adj_x, adj_y, z, map_width, map_height)];
						switch (tile_id) {
							case 1: {
								if (redraw_buffer) {
									blit_surface_to_click_buffer(wall_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
								}
								SDL_RenderCopy(renderer, wall_tex, NULL, &dest);
							} break;
							case 2: {
								if (redraw_buffer) {
									blit_surface_to_click_buffer(grass_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
								}
								SDL_RenderCopy(renderer, grass_tex, NULL, &dest);
							} break;
							case 3: {
								if (redraw_buffer) {
									blit_surface_to_click_buffer(brick_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
								}
								SDL_RenderCopy(renderer, brick_tex, NULL, &dest);
							} break;
							case 4: {
								if (redraw_buffer) {
									blit_surface_to_click_buffer(wood_wall_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
								}
								SDL_RenderCopy(renderer, wood_wall_tex, NULL, &dest);
							} break;
							case 5: {
								if (redraw_buffer) {
									blit_surface_to_click_buffer(dir_door_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
								}
								SDL_RenderCopy(renderer, dir_door_tex, NULL, &dest);
							} break;
							case 6: {
								if (redraw_buffer) {
									blit_surface_to_click_buffer(roof_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
								}
								SDL_RenderCopy(renderer, roof_tex, NULL, &dest);
							} break;
							case 7: {
								if (redraw_buffer) {
									blit_surface_to_click_buffer(cylinder_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
								}
								SDL_RenderCopy(renderer, cylinder_tex, NULL, &dest);
							} break;
							case 8: {
								if (redraw_buffer) {
									blit_surface_to_click_buffer(white_brick_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
								}
								SDL_RenderCopy(renderer, white_brick_tex, NULL, &dest);
							} break;
							case 9: {
								if (redraw_buffer) {
									blit_surface_to_click_buffer(wood_ladder_wall_bmp, &dest, click_map, screen_width, screen_height, threed_to_oned(adj_x, adj_y, z, map_width, map_height));
								}
								SDL_RenderCopy(renderer, wood_ladder_wall_tex, NULL, &dest);
							} break;
						}
					}
				}
			}

			redraw_buffer = 0;
			SDL_RenderPresent(renderer);
		} else {
			SDL_Delay(50);
		}
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
