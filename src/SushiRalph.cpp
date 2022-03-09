#include "SushiRalph.h"
#include "render.cpp"

internal inline f32 lerp(f32 a, f32 b, f32 t) { return a * (1.0f - t) + b * t; }
internal inline vf3 lerp(vf3 a, vf3 b, f32 t) { return a * (1.0f - t) + b * t; }
internal inline vf4 lerp(vf4 a, vf4 b, f32 t) { return a * (1.0f - t) + b * t; }

internal inline f32 dampen(f32 a, f32 b, f32 k, f32 dt) { return lerp(a, b, 1.0f - expf(-k * dt)); }
internal inline vf3 dampen(vf3 a, vf3 b, f32 k, f32 dt) { return lerp(a, b, 1.0f - expf(-k * dt)); }
internal inline vf4 dampen(vf4 a, vf4 b, f32 k, f32 dt) { return lerp(a, b, 1.0f - expf(-k * dt)); }

internal bool32 collide_rect_rect(f32* result, vf2 ray, vf2 bottom_left_a, vf2 dimensions_a, vf2 bottom_left_b, vf2 dimensions_b)
{
	vf2 delta = bottom_left_b - bottom_left_a;
	f32 ts[] =
		{
			(delta.x - dimensions_a.x) / ray.x,
			(delta.x + dimensions_b.x) / ray.x,
			(delta.y - dimensions_a.y) / ray.y,
			(delta.y + dimensions_b.y) / ray.y
		};

	bool32 bs[] =
		{
			ts[0] >= 0.0f && IN_RANGE(ts[0] * ray.y - delta.y, -dimensions_a.y, dimensions_b.y),
			ts[1] >= 0.0f && IN_RANGE(ts[1] * ray.y - delta.y, -dimensions_a.y, dimensions_b.y),
			ts[2] >= 0.0f && IN_RANGE(ts[2] * ray.x - delta.x, -dimensions_a.x, dimensions_b.x),
			ts[3] >= 0.0f && IN_RANGE(ts[3] * ray.x - delta.x, -dimensions_a.x, dimensions_b.x)
		};

	FOR_ELEMS(t, ts)
	{
		if (!bs[t_index])
		{
			*t = INFINITY;
		}
	}

	f32 t = MINIMUM(MINIMUM(ts[0], ts[1]), MINIMUM(ts[2], ts[3]));

	if (t == INFINITY || t > 1.0f)
	{
		return false;
	}
	else
	{
		*result = t;
		return true;
	}
}

internal Sprite load_sprite(SDL_Renderer* renderer, strlit file_path, f32 scalar, vf2 offset, i32 frame_count = 1, f32 seconds_per_frame = 0.0f)
{
	SDL_Surface* sprite_surface = SDL_LoadBMP(file_path);
	DEFER { SDL_FreeSurface(sprite_surface); };
	ASSERT(sprite_surface);

	Sprite sprite;

	sprite.texture = SDL_CreateTextureFromSurface(renderer, sprite_surface);
	SDL_SetTextureBlendMode(sprite.texture, SDL_BLENDMODE_BLEND);

	SDL_QueryTexture(sprite.texture, 0, 0, &sprite.width_pixels, &sprite.height_pixels);
	sprite.width_pixels /= frame_count;

	sprite.scalar              = scalar;
	sprite.offset              = offset;
	sprite.frame_index         = 0;
	sprite.frame_count         = frame_count;
	sprite.seconds_per_frame   = seconds_per_frame;
	sprite.seconds_accumulated = 0.0f;

	return sprite;
}

internal void age_sprite(Sprite* sprite, f32 dt)
{
	sprite->seconds_accumulated += dt;
	if (sprite->seconds_accumulated >= sprite->seconds_per_frame)
	{
		sprite->seconds_accumulated -= sprite->seconds_per_frame;
		sprite->frame_index = (sprite->frame_index + 1) % sprite->frame_count;
	}
}

extern "C" PROTOTYPE_INITIALIZE(initialize)
{
	State* state = reinterpret_cast<State*>(program->memory);

	*state = {};
	state->type = StateType::title_menu;
}

extern "C" PROTOTYPE_BOOT_UP(boot_up)
{
	State* state = reinterpret_cast<State*>(program->memory);

	state->font = FC_CreateFont();
	FC_LoadFont(state->font, program->renderer, "C:/code/misc/fonts/Consolas.ttf", 64, { 255, 255, 255, 255 }, TTF_STYLE_NORMAL);

	state->ralph_running_sprite = load_sprite(program->renderer, "W:/data/ralph_running.bmp", 0.6f, { 0.5f, 0.5f }, 4, 0.25f);
	state->sushi_sprite         = load_sprite(program->renderer, "W:/data/sushi.bmp", 0.1f, { 0.5f, 0.5f });
}

extern "C" PROTOTYPE_BOOT_DOWN(boot_down)
{
	State* state = reinterpret_cast<State*>(program->memory);

	FC_FreeFont(state->font);
	SDL_DestroyTexture(state->ralph_running_sprite.texture);
	SDL_DestroyTexture(state->sushi_sprite.texture);
}

extern "C" PROTOTYPE_UPDATE(update)
{
	State* state = reinterpret_cast<State*>(program->memory);

	for (SDL_Event event; SDL_PollEvent(&event);)
	{
		switch (event.type)
		{
			case SDL_WINDOWEVENT:
			{
				if (event.window.event == SDL_WINDOWEVENT_CLOSE)
				{
					program->is_running = false;
					return;
				}
			} break;

			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				if (!event.key.repeat)
				{
					// @TODO@ Presses between frames can get lost.
					switch (event.key.keysym.sym)
					{
						case SDLK_a      : case SDLK_LEFT  : state->input.left   = event.key.state == SDL_PRESSED; break;
						case SDLK_d      : case SDLK_RIGHT : state->input.right  = event.key.state == SDL_PRESSED; break;
						case SDLK_s      : case SDLK_DOWN  : state->input.down   = event.key.state == SDL_PRESSED; break;
						case SDLK_w      : case SDLK_UP    : state->input.up     = event.key.state == SDL_PRESSED; break;
						case SDLK_RETURN : case SDLK_SPACE : state->input.accept = event.key.state == SDL_PRESSED; break;
					}
				}
			}
		}
	}

	state->seconds_accumulated += program->delta_seconds;

	if (state->seconds_accumulated >= SECONDS_PER_UPDATE)
	{
		state->seconds_accumulated = 0.0f; // @STICKY@ Lose lagged frames.

		//
		// Update.
		//

		switch (state->type)
		{
			case StateType::title_menu:
			{
				// @TODO@ Make holding down work nicely.
				if (state->input.left && !state->prev_input.left && state->title_menu.option_index > 0)
				{
					--state->title_menu.option_index;
				}
				if (state->input.right && !state->prev_input.right && state->title_menu.option_index < ARRAY_CAPACITY(TITLE_MENU_OPTIONS) - 1)
				{
					++state->title_menu.option_index;
				}

				if (state->input.accept && !state->prev_input.accept)
				{
					switch (state->title_menu.option_index)
					{
						case 0:
						{
							state->type                      = StateType::playing;
							state->playing.ralph_belt_index  = 1;
							state->playing.ralph_position    = { 250.0f, (state->playing.ralph_belt_index + 0.5f) * BELT_HEIGHT };
							state->playing.obstacle_position = { 750.0f, (1.0f + 0.5f) * BELT_HEIGHT };
						} break;

						case 1:
						{
						} break;

						case 2:
						{
						} break;

						case 3:
						{
							program->is_running = false;
							return;
						} break;
					}
				}

				state->belt_offsets[1] = dampen(state->belt_offsets[1], -state->title_menu.option_index * TITLE_MENU_OPTION_SPACING, 16.0f, SECONDS_PER_UPDATE);
			} break;

			case StateType::playing:
			{
				// @TODO@ Make holding down work nicely.
				if (state->input.down && !state->prev_input.down && state->playing.ralph_belt_index > 0)
				{
					--state->playing.ralph_belt_index;
					state->playing.ralph_position = { 250.0f, (state->playing.ralph_belt_index + 0.5f) * BELT_HEIGHT };
				}
				if (state->input.up && !state->prev_input.up && state->playing.ralph_belt_index < 2)
				{
					++state->playing.ralph_belt_index;
					state->playing.ralph_position = { 250.0f, (state->playing.ralph_belt_index + 0.5f) * BELT_HEIGHT };
				}

				f32 collide_t;
				if (collide_rect_rect(&collide_t, vf2 { BELT_SPEED, 0.0f } * SECONDS_PER_UPDATE, state->playing.ralph_position - RALPH_HITBOX_DIMENSIONS / 2.0f, RALPH_HITBOX_DIMENSIONS, state->playing.obstacle_position - OBSTACLE_HITBOX_DIMENSIONS / 2.0f, OBSTACLE_HITBOX_DIMENSIONS))
				{
					state->type                    = StateType::game_over;
					state->playing.ralph_position += vf2 { BELT_SPEED, 0.0f } * SECONDS_PER_UPDATE * collide_t;

					FOR_ELEMS(it, state->belt_offsets)
					{
						*it -= BELT_SPEED * SECONDS_PER_UPDATE * collide_t;
					}

					state->playing.obstacle_position.x -= BELT_SPEED * SECONDS_PER_UPDATE * collide_t;

					age_sprite(&state->ralph_running_sprite, SECONDS_PER_UPDATE * collide_t);
				}
				else
				{
					FOR_ELEMS(it, state->belt_offsets)
					{
						*it -= BELT_SPEED * SECONDS_PER_UPDATE;
					}

					state->playing.obstacle_position.x -= BELT_SPEED * SECONDS_PER_UPDATE;

					age_sprite(&state->ralph_running_sprite, SECONDS_PER_UPDATE);
				}
			} break;
		}

		//
		// Render.
		//

		set_color(program->renderer, {});
		SDL_RenderClear(program->renderer);

		FOR_RANGE(belt_index, 3)
		{
			set_color(program->renderer, monochrome(BELT_LIGHTNESS[belt_index]));
			draw_rect(program->renderer, { 0.0f, belt_index * BELT_HEIGHT }, { WINDOW_DIMENSIONS.x, BELT_HEIGHT });

			set_color(program->renderer, monochrome(0.5f));
			FOR_RANGE(scale_index, static_cast<i32>(WINDOW_DIMENSIONS.x / BELT_SPACING) + 2)
			{
				vf2 mid = { fmodf(state->belt_offsets[belt_index], BELT_SPACING) + (scale_index - 1.0f) * BELT_SPACING, (belt_index + 0.5f) * BELT_HEIGHT };
				draw_line(program->renderer, mid, mid + vf2 { BELT_SPACING, -BELT_HEIGHT / 2.0f });
				draw_line(program->renderer, mid, mid + vf2 { BELT_SPACING,  BELT_HEIGHT / 2.0f });
			}
		}

		set_color(program->renderer, { 1.0f, 1.0f, 1.0f, 1.0f });
		draw_line(program->renderer, { 0.0f, WINDOW_DIMENSIONS.y        / 3.0f }, { WINDOW_DIMENSIONS.x, WINDOW_DIMENSIONS.y        / 3.0f });
		draw_line(program->renderer, { 0.0f, WINDOW_DIMENSIONS.y * 2.0f / 3.0f }, { WINDOW_DIMENSIONS.x, WINDOW_DIMENSIONS.y * 2.0f / 3.0f });

		switch (state->type)
		{
			case StateType::title_menu:
			{
				draw_text(program->renderer, state->font, WINDOW_DIMENSIONS / 2.0f + vf2 { 0.0f, BELT_HEIGHT - FC_GetBaseline(state->font) / 2.0f }, FC_ALIGN_CENTER, 1.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, "Sushi Ralph");

				FOR_ELEMS(it, TITLE_MENU_OPTIONS)
				{
					draw_text(program->renderer, state->font, WINDOW_DIMENSIONS / 2.0f + vf2 { it_index * TITLE_MENU_OPTION_SPACING + state->belt_offsets[1], -10.0f }, FC_ALIGN_CENTER, 0.7f, { 1.0f, 1.0f, 1.0f, 1.0f }, "%s", *it);
				}
			} break;

			case StateType::playing:
			case StateType::game_over:
			{
				set_color(program->renderer, { 0.0f, 1.0f, 0.0f, 1.0f });
				draw_rect(program->renderer, state->playing.ralph_position - RALPH_HITBOX_DIMENSIONS / 2.0f, RALPH_HITBOX_DIMENSIONS);

				set_color(program->renderer, { 1.0f, 1.0f, 0.0f, 1.0f });
				draw_rect(program->renderer, state->playing.obstacle_position - OBSTACLE_HITBOX_DIMENSIONS / 2.0f, OBSTACLE_HITBOX_DIMENSIONS);

				draw_sprite(program->renderer, &state->ralph_running_sprite, state->playing.ralph_position);
				draw_sprite(program->renderer, &state->sushi_sprite, state->playing.obstacle_position);

				set_color(program->renderer, { 1.0f, 0.0f, 0.0f, 1.0f });
				draw_crosshair(program->renderer, state->playing.ralph_position, 25.0f);

				set_color(program->renderer, { 1.0f, 1.0f, 0.0f, 1.0f });
				draw_crosshair(program->renderer, state->playing.obstacle_position, 25.0f);
			} break;
		}

		SDL_RenderPresent(program->renderer);

		state->prev_input = state->input;
	}
}
