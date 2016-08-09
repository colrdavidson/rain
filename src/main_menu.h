#ifndef MAIN_MENU_H
#define MAIN_MENU_H

typedef struct MainMenu {
	TTF_Font *button_font;
	TTF_Font *title_font;
	SDL_Color font_color;

	SDL_Surface *start_surf;
	SDL_Surface *options_surf;
	SDL_Surface *exit_surf;
	SDL_Surface *fullscreen_surf;
	SDL_Surface *windowed_surf;
	SDL_Surface *menu_anim_surf;
	SDL_Surface *start_bg_surf;
	SDL_Surface *rain_surf;

	SDL_Texture *start_tex;
	SDL_Texture *options_tex;
	SDL_Texture *exit_tex;
	SDL_Texture *fullscreen_tex;
	SDL_Texture *windowed_tex;
	SDL_Texture *menu_anim_tex;
	SDL_Texture *start_bg_tex;
	SDL_Texture *rain_tex;

	u32 max_drops;
	f32 drop_speed;
	SDL_Rect *raindrops;

	u32 max_walkers;
	f32 walk_speed;
	SDL_Rect *walkers;

	u8 fullscreen;
	u8 menu_animation;
	u8 options_selected;
} MainMenu;

MainMenu *init_main_menu(Game *game) {
	MainMenu *m = malloc(sizeof(MainMenu));

	m->button_font = TTF_OpenFont("assets/greenscr.ttf", 48);
	m->title_font = TTF_OpenFont("assets/din_reg.ttf", 200);
	SDL_Color c = { 255, 255, 255, 255 };
	m->font_color = c;

	m->start_surf = TTF_RenderText_Solid(m->button_font, "Start", m->font_color);
	m->options_surf = TTF_RenderText_Solid(m->button_font, "Options", m->font_color);
	m->exit_surf = TTF_RenderText_Solid(m->button_font, "Exit", m->font_color);
	m->fullscreen_surf = TTF_RenderText_Solid(m->button_font, "Fullscreen", m->font_color);
	m->windowed_surf = TTF_RenderText_Solid(m->button_font, "Windowed", m->font_color);
	m->menu_anim_surf = TTF_RenderText_Solid(m->button_font, "Animation", m->font_color);

	m->start_tex = SDL_CreateTextureFromSurface(game->renderer, m->start_surf);
	m->options_tex = SDL_CreateTextureFromSurface(game->renderer, m->options_surf);
	m->exit_tex = SDL_CreateTextureFromSurface(game->renderer, m->exit_surf);
	m->fullscreen_tex = SDL_CreateTextureFromSurface(game->renderer, m->fullscreen_surf);
	m->windowed_tex = SDL_CreateTextureFromSurface(game->renderer, m->windowed_surf);
	m->menu_anim_tex = SDL_CreateTextureFromSurface(game->renderer, m->menu_anim_surf);

	m->rain_surf = TTF_RenderText_Solid(m->title_font, "Rain", m->font_color);
	m->rain_tex = SDL_CreateTextureFromSurface(game->renderer, m->rain_surf);

	m->start_bg_surf = IMG_Load("assets/beat.png");
	m->start_bg_tex = SDL_CreateTextureFromSurface(game->renderer, m->start_bg_surf);

	m->max_drops = 300;
	m->drop_speed = 2.0;
	m->raindrops = malloc(sizeof(SDL_Rect) * m->max_drops);
	for (u32 i = 0; i < m->max_drops; i++) {
		m->raindrops[i].x = ((f32)game->screen_width / (f32)m->max_drops) * (rand() % m->max_drops);
		m->raindrops[i].y = -1 * (rand() % 1000);
		m->raindrops[i].w = 2;
		m->raindrops[i].h = 30;
	}

	m->max_walkers = 8;
	m->walk_speed = 2.5;
	m->walkers = malloc(sizeof(SDL_Rect) * m->max_walkers);
	for (u32 i = 0; i < m->max_walkers; i++) {
		m->walkers[i].x = ((f32)game->screen_width / (f32)m->max_walkers) * i;
		m->walkers[i].y = game->screen_height - (((f32)game->screen_height / 5.0) * 1.70);
		m->walkers[i].w = 10;
		m->walkers[i].h = 30;
	}

	m->options_selected = false;
	m->menu_animation = true;
	m->fullscreen = false;

	return m;
}

void handle_main_menu_events(MainMenu *main_menu, Game *game) {
	SDL_Event event;

	SDL_PumpEvents();
	const u8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_UP]) {
		game->redraw_buffer = true;
	}
	if (state[SDL_SCANCODE_DOWN]) {
		game->redraw_buffer = true;
	}
	if (state[SDL_SCANCODE_LEFT]) {
		game->redraw_buffer = true;
	}
	if (state[SDL_SCANCODE_RIGHT]) {
		game->redraw_buffer = true;
	}

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_KEYDOWN: {
				switch (event.key.keysym.sym) {
				}
			} break;
			case SDL_MOUSEBUTTONDOWN: {
				i32 mouse_x, mouse_y;
				u32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

				mouse_x *= game->dpi_ratio;
				mouse_y *= game->dpi_ratio;

				if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
					u16 tile_id = game->click_map[twod_to_oned(mouse_x, mouse_y, game->screen_width)];

					switch (tile_id) {
						case 0: { } break;
						case 1: {
							game->cur_state = RainGameState;
							game->transition = true;
							game->redraw_buffer = true;
						} break;
						case 2: {
							main_menu->options_selected = true;
							game->redraw_buffer = true;
						} break;
						case 3: {
							game->running = false;
						} break;
						case 4: {
							main_menu->options_selected = false;
							game->redraw_buffer = true;
						} break;
						case 5: {
							if (!main_menu->fullscreen) {
								SDL_SetWindowFullscreen(game->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
								main_menu->fullscreen = true;
							} else {
								SDL_SetWindowFullscreen(game->window, 0);
								main_menu->fullscreen = false;
							}
							game->redraw_buffer = true;
						} break;
						case 6: {
							main_menu->menu_animation = !main_menu->menu_animation;
						} break;
						default: {
							printf("clicked: %u\n", tile_id);
						};
					}
				} else if (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
				}

			} break;
			case SDL_WINDOWEVENT: {
				switch (event.window.event) {
					case SDL_WINDOWEVENT_RESIZED: {
						printf("resized! %d, %d\n", event.window.data1, event.window.data2);
						i32 tmp_width;
						i32 tmp_height;
						SDL_GetRendererOutputSize(game->renderer, &tmp_width, &tmp_height);
						f32 rescale_x = (f32)tmp_width / (f32)game->screen_width;
						f32 rescale_y = (f32)tmp_height / (f32)game->screen_height;
						game->screen_width = tmp_width;
						game->screen_height = tmp_height;

						wipe_clickbuffer(game);

						for (u32 i = 0; i < main_menu->max_drops; i++) {
							main_menu->raindrops[i].x = ((f32)game->screen_width / (f32)main_menu->max_drops) * (rand() % main_menu->max_drops);
							main_menu->raindrops[i].w = (f32)main_menu->raindrops[i].w * rescale_x;
							main_menu->raindrops[i].h = (f32)main_menu->raindrops[i].h * rescale_y;
						}
						main_menu->drop_speed *= rescale_y;

						for (u32 i = 0; i < main_menu->max_walkers; i++) {
							main_menu->walkers[i].x = ((f32)game->screen_width / (f32)main_menu->max_walkers) * (rand() % main_menu->max_walkers);
							main_menu->walkers[i].y = (f32)main_menu->walkers[i].y * rescale_y;
							main_menu->walkers[i].w = (f32)main_menu->walkers[i].w * rescale_x;
							main_menu->walkers[i].h = (f32)main_menu->walkers[i].h * rescale_y;
						}
						main_menu->walk_speed *= rescale_x;

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

void render_menu(MainMenu *m, Game* game, f32 dt) {
	SDL_Rect UI_Frame;
	UI_Frame.w = game->screen_width;
	UI_Frame.h = (f32)game->screen_height / 6.0f;
	UI_Frame.x = 0;
	UI_Frame.y = game->screen_height - UI_Frame.h;

	SDL_Rect StartBackground;
	StartBackground.w = game->screen_width;
	StartBackground.h = game->screen_height - UI_Frame.h;
	StartBackground.x = 0;
	StartBackground.y = 0;

	SDL_Rect StartButton;
	StartButton.w = (f32)UI_Frame.w / 5.0f;
	StartButton.h = (f32)UI_Frame.h / 1.5f;
	StartButton.x = ((f32)UI_Frame.w / 4.0f) - ((f32)StartButton.w / 2.0f);
	StartButton.y = (f32)UI_Frame.y + ((f32)UI_Frame.h / 2.0f) - ((f32)StartButton.h / 2.0f);

	SDL_Rect OptionsButton;
	OptionsButton.w = StartButton.w;
	OptionsButton.h = StartButton.h;
	OptionsButton.x = ((f32)UI_Frame.w / 2.0f) - ((f32)OptionsButton.w / 2.0f);
	OptionsButton.y = StartButton.y;

	SDL_Rect ExitButton;
	ExitButton.w = StartButton.w;
	ExitButton.h = StartButton.h;
	ExitButton.x = (((f32)UI_Frame.w / 4.0f) * 3.0f) - ((f32)ExitButton.w / 2.0f);
	ExitButton.y = StartButton.y;

	SDL_Rect StartButtonText;
	StartButtonText.w = m->start_surf->clip_rect.w;
	StartButtonText.h = m->start_surf->clip_rect.h;
	StartButtonText.x = StartButton.x + ((f32)StartButton.w / 2.0f) - ((f32)StartButtonText.w / 2.0f);
	StartButtonText.y = StartButton.y + ((f32)StartButton.h / 2.0f) - ((f32)StartButtonText.h / 2.0f);

	SDL_Rect OptionsButtonText;
	OptionsButtonText.w = m->options_surf->clip_rect.w;
	OptionsButtonText.h = m->options_surf->clip_rect.h;
	OptionsButtonText.x = OptionsButton.x + ((f32)OptionsButton.w / 2.0f) - ((f32)OptionsButtonText.w / 2.0f);
	OptionsButtonText.y = OptionsButton.y + ((f32)OptionsButton.h / 2.0f) - ((f32)OptionsButtonText.h / 2.0f);

	SDL_Rect ExitButtonText;
	ExitButtonText.w = m->exit_surf->clip_rect.w;
	ExitButtonText.h = m->exit_surf->clip_rect.h;
	ExitButtonText.x = ExitButton.x + ((f32)ExitButton.w / 2.0f) - ((f32)ExitButtonText.w / 2.0f);
	ExitButtonText.y = ExitButton.y + ((f32)ExitButton.h / 2.0f) - ((f32)ExitButtonText.h / 2.0f);

	SDL_Rect RainTitle;
	RainTitle.w = m->rain_surf->clip_rect.w;
	RainTitle.h = m->rain_surf->clip_rect.h;
	RainTitle.x = (game->screen_width / 2.0f) - ((f32)RainTitle.w / 2.0f);
	RainTitle.y = (game->screen_height / 4.0f) - ((f32)RainTitle.h / 2.0f);

	SDL_RenderCopy(game->renderer, m->start_bg_tex, NULL, &StartBackground);
	SDL_RenderCopy(game->renderer, m->rain_tex, NULL, &RainTitle);

	SDL_SetRenderDrawColor(game->renderer, 50, 50, 175, 220);
	for (u32 i = 0; i < m->max_drops; i++) {
		if (m->menu_animation) {
			if (m->raindrops[i].y > UI_Frame.y - ((f32)UI_Frame.h / 2.0)) {
				m->raindrops[i].y = -(rand() % 1000);
			} else {
				m->raindrops[i].y += 9.8 * m->drop_speed * dt;
			}
		}

		SDL_RenderFillRect(game->renderer, &m->raindrops[i]);
	}

	SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
	for (u32 i = 0; i < m->max_walkers; i++) {
		if (m->menu_animation) {
			if (i > m->max_walkers / 2) {
				if (m->walkers[i].x > game->screen_width) {
					m->walkers[i].x = 0;
				} else {
					m->walkers[i].x += (m->walk_speed * dt) * (rand() % 5);
				}
			} else {
				if (m->walkers[i].x < 0) {
					m->walkers[i].x = game->screen_width;
				} else {
					m->walkers[i].x -= (m->walk_speed * dt) * (rand() % 5);
				}
			}
		}

		SDL_RenderFillRect(game->renderer, &m->walkers[i]);
	}

	SDL_SetRenderDrawColor(game->renderer, 100, 100, 100, 255);
	SDL_RenderFillRect(game->renderer, &UI_Frame);

	SDL_SetRenderDrawColor(game->renderer, 60, 60, 60, 255);
	SDL_RenderFillRect(game->renderer, &StartButton);
	SDL_RenderFillRect(game->renderer, &OptionsButton);
	SDL_RenderFillRect(game->renderer, &ExitButton);

	SDL_RenderCopy(game->renderer, m->start_tex, NULL, &StartButtonText);
	SDL_RenderCopy(game->renderer, m->options_tex, NULL, &OptionsButtonText);
	SDL_RenderCopy(game->renderer, m->exit_tex, NULL, &ExitButtonText);

	if (game->redraw_buffer) {
		blit_rect_to_click_buffer(&StartButton, game->click_map, game->screen_width, game->screen_height, 1);
		blit_rect_to_click_buffer(&OptionsButton, game->click_map, game->screen_width, game->screen_height, 2);
		blit_rect_to_click_buffer(&ExitButton, game->click_map, game->screen_width, game->screen_height, 3);
	}

	if (m->options_selected) {
		SDL_Rect Options_Frame;
		Options_Frame.w = (f32)StartBackground.w * 0.75f;
		Options_Frame.h = (f32)StartBackground.h * 0.75f;
		Options_Frame.x = ((f32)StartBackground.w / 2.0f) - ((f32)Options_Frame.w / 2.0f);
		Options_Frame.y = ((f32)StartBackground.h / 2.0f) - ((f32)Options_Frame.h / 2.0f);

		SDL_Rect CloseOptionsButton;
		CloseOptionsButton.w = (f32)game->screen_width / 32.0f;
		CloseOptionsButton.h = CloseOptionsButton.w;
		CloseOptionsButton.x = Options_Frame.x;
		CloseOptionsButton.y = Options_Frame.y;

		SDL_Rect FullscreenButton;
		FullscreenButton.w = (f32)Options_Frame.w / 2.5f;
		FullscreenButton.h = StartButton.h;
		FullscreenButton.x = Options_Frame.x + Options_Frame.w - FullscreenButton.w - ((f32)FullscreenButton.w / 8.0f);
		FullscreenButton.y = Options_Frame.y + ((f32)FullscreenButton.w / 8.0f);

		SDL_Rect MenuAnimationButton;
		MenuAnimationButton.w = FullscreenButton.w;
		MenuAnimationButton.h = FullscreenButton.h;
		MenuAnimationButton.x = FullscreenButton.x;
		MenuAnimationButton.y = FullscreenButton.y + FullscreenButton.h + ((f32)FullscreenButton.w / 8.0f);

		SDL_Rect FullscreenButtonText;
		FullscreenButtonText.w = m->fullscreen_surf->clip_rect.w;
		FullscreenButtonText.h = m->fullscreen_surf->clip_rect.h;
		FullscreenButtonText.x = FullscreenButton.x + ((f32)FullscreenButton.w / 2.0f) - ((f32)FullscreenButtonText.w / 2.0f);
		FullscreenButtonText.y = FullscreenButton.y + ((f32)FullscreenButton.h / 2.0f) - ((f32)FullscreenButtonText.h / 2.0f);

		SDL_Rect WindowedButtonText;
		WindowedButtonText.w = m->windowed_surf->clip_rect.w;
		WindowedButtonText.h = m->windowed_surf->clip_rect.h;
		WindowedButtonText.x = FullscreenButton.x + ((f32)FullscreenButton.w / 2.0f) - ((f32)WindowedButtonText.w / 2.0f);
		WindowedButtonText.y = FullscreenButton.y + ((f32)FullscreenButton.h / 2.0f) - ((f32)WindowedButtonText.h / 2.0f);

		SDL_Rect MenuAnimationButtonText;
		MenuAnimationButtonText.w = m->menu_anim_surf->clip_rect.w;
		MenuAnimationButtonText.h = m->menu_anim_surf->clip_rect.h;
		MenuAnimationButtonText.x = MenuAnimationButton.x + ((f32)MenuAnimationButton.w / 2.0f) - ((f32)MenuAnimationButtonText.w / 2.0f);
		MenuAnimationButtonText.y = MenuAnimationButton.y + ((f32)MenuAnimationButton.h / 2.0f) - ((f32)MenuAnimationButtonText.h / 2.0f);

		SDL_SetRenderDrawColor(game->renderer, 80, 80, 80, 240);
		SDL_RenderFillRect(game->renderer, &Options_Frame);

		SDL_SetRenderDrawColor(game->renderer, 150, 50, 50, 255);
		SDL_RenderFillRect(game->renderer, &CloseOptionsButton);

		SDL_SetRenderDrawColor(game->renderer, 60, 60, 60, 255);
		SDL_RenderFillRect(game->renderer, &FullscreenButton);
		SDL_RenderFillRect(game->renderer, &MenuAnimationButton);

		if (!m->fullscreen) {
			SDL_RenderCopy(game->renderer, m->fullscreen_tex, NULL, &FullscreenButtonText);
		} else {
			SDL_RenderCopy(game->renderer, m->windowed_tex, NULL, &WindowedButtonText);
		}

		SDL_RenderCopy(game->renderer, m->menu_anim_tex, NULL, &MenuAnimationButtonText);

		if (game->redraw_buffer) {
			blit_rect_to_click_buffer(&CloseOptionsButton, game->click_map, game->screen_width, game->screen_height, 4);
			blit_rect_to_click_buffer(&FullscreenButton, game->click_map, game->screen_width, game->screen_height, 5);
			blit_rect_to_click_buffer(&MenuAnimationButton, game->click_map, game->screen_width, game->screen_height, 6);
		}
	}
}

#endif
