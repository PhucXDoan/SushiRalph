#include "SushiRalph.h"
#include "render.cpp"

internal inline f32 square(f32 x) { return x * x; }

internal inline f32 lerp(f32 a, f32 b, f32 t) { return a * (1.0f - t) + b * t; }
internal inline vf3 lerp(vf3 a, vf3 b, f32 t) { return a * (1.0f - t) + b * t; }
internal inline vf4 lerp(vf4 a, vf4 b, f32 t) { return a * (1.0f - t) + b * t; }

internal inline f32 dampen(f32 a, f32 b, f32 k, f32 dt) { return lerp(a, b, 1.0f - expf(-k * dt)); }
internal inline vf3 dampen(vf3 a, vf3 b, f32 k, f32 dt) { return lerp(a, b, 1.0f - expf(-k * dt)); }
internal inline vf4 dampen(vf4 a, vf4 b, f32 k, f32 dt) { return lerp(a, b, 1.0f - expf(-k * dt)); }

internal inline f32 sigmoid(f32 x, f32 k)
{
	return 1.0f / (1.0f + expf(-x * k));
}

internal inline f32 ease_in(f32 x)
{
	return sinf(TAU / 4.0f * x);
}

internal inline f32 rng(u32* seed)
{
	return RAND_TABLE[++*seed % ARRAY_CAPACITY(RAND_TABLE)] / 65536.0f;
}

internal inline i32 rng(u32* seed, i32 start, i32 end)
{
	return static_cast<i32>(rng(seed) * (end - start) + start);
}

internal inline f32 rng(u32* seed, f32 start, f32 end)
{
	return rng(seed) * (end - start) + start;
}

internal bool32 collide_rect_rect(f32* result, vf3 ray, vf3 corner_a, vf3 dimensions_a, vf3 corner_b, vf3 dimensions_b)
{
	vf3 delta = corner_b - corner_a;
	f32 ts[] =
		{
			(delta.x - dimensions_a.x) / ray.x,
			(delta.x + dimensions_b.x) / ray.x,
			(delta.y - dimensions_a.y) / ray.y,
			(delta.y + dimensions_b.y) / ray.y,
			(delta.z - dimensions_a.z) / ray.z,
			(delta.z + dimensions_b.z) / ray.z
		};

	bool32 bs[] =
		{
			ts[0] >= 0.0f && IN_RANGE(ts[0] * ray.y - delta.y, -dimensions_a.y, dimensions_b.y) && IN_RANGE(ts[0] * ray.z - delta.z, -dimensions_a.z, dimensions_b.z),
			ts[1] >= 0.0f && IN_RANGE(ts[1] * ray.y - delta.y, -dimensions_a.y, dimensions_b.y) && IN_RANGE(ts[1] * ray.z - delta.z, -dimensions_a.z, dimensions_b.z),
			ts[2] >= 0.0f && IN_RANGE(ts[2] * ray.x - delta.x, -dimensions_a.x, dimensions_b.x) && IN_RANGE(ts[2] * ray.z - delta.z, -dimensions_a.z, dimensions_b.z),
			ts[3] >= 0.0f && IN_RANGE(ts[3] * ray.x - delta.x, -dimensions_a.x, dimensions_b.x) && IN_RANGE(ts[3] * ray.z - delta.z, -dimensions_a.z, dimensions_b.z),
			ts[4] >= 0.0f && IN_RANGE(ts[4] * ray.x - delta.x, -dimensions_a.x, dimensions_b.x) && IN_RANGE(ts[4] * ray.y - delta.y, -dimensions_a.y, dimensions_b.y),
			ts[5] >= 0.0f && IN_RANGE(ts[5] * ray.x - delta.x, -dimensions_a.x, dimensions_b.x) && IN_RANGE(ts[5] * ray.y - delta.y, -dimensions_a.y, dimensions_b.y)
		};

	f32 t = INFINITY;

	FOR_ELEMS(it, ts)
	{
		if (bs[it_index] && *it < t)
		{
			t = *it;
		}
	}

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
		sprite->seconds_accumulated = 0;
		sprite->frame_index = MINIMUM(sprite->frame_index + 1, sprite->frame_count);
	}
}

internal void loop_sprite(Sprite* sprite, f32 dt)
{
	sprite->seconds_accumulated += dt;
	if (sprite->seconds_accumulated >= sprite->seconds_per_frame)
	{
		sprite->seconds_accumulated = 0;
		sprite->frame_index = (sprite->frame_index + 1) % sprite->frame_count;
	}
}

extern "C" PROTOTYPE_INITIALIZE(initialize)
{
	State* state = reinterpret_cast<State*>(program->memory);

	*state = {};
	state->type                         = StateType::title_menu;
	state->title_menu.resetting_keytime = 1.0f;
}

extern "C" PROTOTYPE_BOOT_UP(boot_up)
{
	State* state = reinterpret_cast<State*>(program->memory);

	state->font = FC_CreateFont();
	FC_LoadFont(state->font, program->renderer, "C:/code/misc/fonts/Consolas.ttf", 64, { 255, 255, 255, 255 }, TTF_STYLE_NORMAL);

	state->ralph_running_sprite   = load_sprite(program->renderer, "W:/data/ralph_running.bmp", 0.6f, { 0.5f, 0.4f }, 4, 0.25f);
	state->ralph_exploding_sprite = load_sprite(program->renderer, "W:/data/ralph_exploding.bmp", 0.6f, { 0.5f, 0.4f }, 4, 0.15f);
	state->sushi_sprite           = load_sprite(program->renderer, "W:/data/sushi.bmp", 0.15f, { 0.5f, 0.6f });

	FILE* save_data;
	errno_t save_data_error = fopen_s(&save_data, SAVE_DATA_FILE_PATH, "rb");

	if (save_data_error == 0)
	{
		fread(&state->highest_calories_burned, 1, sizeof(state->highest_calories_burned), save_data);
		fclose(save_data);
	}
	else
	{
		DEBUG_printf("No save file found at '%s'.\n", SAVE_DATA_FILE_PATH);
	}
}

extern "C" PROTOTYPE_BOOT_DOWN(boot_down)
{
	State* state = reinterpret_cast<State*>(program->memory);

	FILE* save_data;
	errno_t save_data_error = fopen_s(&save_data, SAVE_DATA_FILE_PATH, "wb");

	if (save_data_error == 0)
	{
		fwrite(&state->highest_calories_burned, 1, sizeof(state->highest_calories_burned), save_data);
		fclose(save_data);
	}
	else
	{
		DEBUG_printf("Could not save at '%s'.\n", SAVE_DATA_FILE_PATH);
	}

	FC_FreeFont(state->font);
	SDL_DestroyTexture(state->ralph_running_sprite.texture);
	SDL_DestroyTexture(state->ralph_exploding_sprite.texture);
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
				if (state->title_menu.resetting_keytime < 1.0f)
				{
					state->title_menu.resetting_keytime += SECONDS_PER_UPDATE / 2.0f;

					if (state->title_menu.resetting_keytime >= 1.0f)
					{
						state->title_menu.resetting_keytime = 1.0f;

						FOR_ELEMS(it, state->belt_offsets)
						{
							*it = 0.0f;
						}
					}
					else
					{
						FOR_ELEMS(it, state->belt_offsets)
						{
							*it = lerp(state->title_menu.initial_belt_offsets[it_index], 0.0f, square(ease_in(state->title_menu.resetting_keytime)));
						}
					}
				}

				if (state->title_menu.resetting_keytime == 1.0f)
				{
					FOR_ELEMS(it, state->dampen_belt_velocities)
					{
						*it = dampen(*it, state->belt_velocities[it_index], 24.0f, SECONDS_PER_UPDATE);
					}

					FOR_ELEMS(it, state->belt_offsets)
					{
						*it += state->dampen_belt_velocities[it_index] * SECONDS_PER_UPDATE;
					}

					// @TODO@ Make holding down work nicely.
					if (state->input.left && !state->prev_input.left && state->title_menu.option_index > 0)
					{
						--state->title_menu.option_index;
					}
					if (state->input.right && !state->prev_input.right && state->title_menu.option_index < ARRAY_CAPACITY(TITLE_MENU_OPTIONS) - 1)
					{
						++state->title_menu.option_index;
					}

					state->belt_velocities[1] = (-state->belt_offsets[1] - state->title_menu.option_index * TITLE_MENU_OPTION_SPACING) * 16.0f;

					if (state->input.accept && !state->prev_input.accept)
					{
						switch (state->title_menu.option_index)
						{
							case 0:
							{
								FOR_ELEMS(it, state->belt_velocities)
								{
									*it = rng(&state->seed, -1.5f, -4.0f);
								}
								FOR_ELEMS(it, state->dampen_belt_velocities)
								{
									*it = 0.0f;
								}

								state->type                        = StateType::playing;
								state->playing                     = {};
								state->playing.ralph_belt_index    = 1;
								state->playing.ralph_position      = { 0.0f, RALPH_HITBOX_DIMENSIONS.y / 2.0f, -1.5f * BELT_HEIGHT };
								state->playing.obstacle_belt_index = rng(&state->seed, 0, 3);
								state->playing.obstacle_hitbox     = { 0.6f, 0.5f, 0.2f };
								state->playing.obstacle_position   = { WINDOW_DIMENSIONS.x / PIXELS_PER_METER + state->playing.obstacle_hitbox.x / 2.0f, state->playing.obstacle_hitbox.y / 2.0f, -(state->playing.obstacle_belt_index + 0.5f) * BELT_HEIGHT };
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
				}
			} break;

			case StateType::playing:
			{
				if (state->playing.intro_keytime < 1.0f)
				{
					state->playing.intro_keytime += SECONDS_PER_UPDATE / 1.0f;

					if (state->playing.intro_keytime >= 1.0f)
					{
						state->playing.intro_keytime    = 1.0f;
						state->playing.ralph_position.x = RALPH_X;
						state->playing.ralph_velocity.x = 0.0f;
					}
					else
					{
						state->playing.ralph_velocity.x = (lerp(0.0f, RALPH_X, ease_in(state->playing.intro_keytime)) - state->playing.ralph_position.x) / SECONDS_PER_UPDATE;
					}
				}

				FOR_ELEMS(it, state->dampen_belt_velocities)
				{
					*it = dampen(*it, state->belt_velocities[it_index], 1.0f, SECONDS_PER_UPDATE);
				}

				if (state->playing.ralph_position.y - RALPH_HITBOX_DIMENSIONS.y / 2.0f <= 0.0f)
				{
					// @TODO@ Make holding down work nicely.
					if (state->input.down && !state->prev_input.down && state->playing.ralph_belt_index > 0)
					{
						--state->playing.ralph_belt_index;
						state->playing.calories_burned += CALORIES_PER_SWITCH;
					}
					if (state->input.up && !state->prev_input.up && state->playing.ralph_belt_index < 2)
					{
						++state->playing.ralph_belt_index;
						state->playing.calories_burned += CALORIES_PER_SWITCH;
					}

					state->ralph_running_sprite.seconds_per_frame = sigmoid(state->playing.ralph_velocity.x - state->dampen_belt_velocities[state->playing.ralph_belt_index], -0.75f);

					state->playing.ralph_velocity.y = 0.0f;
					if (state->input.accept && !state->prev_input.accept)
					{
						state->playing.ralph_velocity.y += 5.0f;
						state->playing.calories_burned  += CALORIES_PER_JUMP;
					}
				}
				else
				{
					state->playing.ralph_velocity.y += GRAVITY * SECONDS_PER_UPDATE;
				}

				state->playing.ralph_velocity.z = ((-state->playing.ralph_belt_index - 0.5f) * BELT_HEIGHT - state->playing.ralph_position.z) * 10.0f;

				vf3 obstacle_velocity =
					{
						state->dampen_belt_velocities[state->playing.obstacle_belt_index],
						0.0f,
						0.0f
					};

				f32 collide_t;
				if
				(
					collide_rect_rect
					(
						&collide_t,
						(state->playing.ralph_velocity - obstacle_velocity) * SECONDS_PER_UPDATE,
						state->playing.ralph_position - RALPH_HITBOX_DIMENSIONS / 2.0f,
						RALPH_HITBOX_DIMENSIONS,
						state->playing.obstacle_position - state->playing.obstacle_hitbox / 2.0f,
						state->playing.obstacle_hitbox
					)
				)
				{
					FOR_ELEMS(it, state->belt_offsets)
					{
						*it += state->dampen_belt_velocities[it_index] * SECONDS_PER_UPDATE * collide_t;
					}

					FOR_ELEMS(it, state->belt_velocities)
					{
						*it = 0.0f;
					}

					state->highest_calories_burned = MAXIMUM(state->highest_calories_burned, state->playing.calories_burned);

					state->ralph_exploding_sprite.frame_index = 0;

					state->playing.ralph_position    += state->playing.ralph_velocity * SECONDS_PER_UPDATE * collide_t;
					state->playing.obstacle_position += obstacle_velocity * SECONDS_PER_UPDATE * collide_t;

					if (state->playing.ralph_position.y - RALPH_HITBOX_DIMENSIONS.y / 2.0f <= 0.0f)
					{
						state->playing.ralph_position.y  = RALPH_HITBOX_DIMENSIONS.y / 2.0f;
						state->playing.calories_burned  += fabsf(state->dampen_belt_velocities[state->playing.ralph_belt_index]) * CALORIES_PER_METER_PER_SECOND * SECONDS_PER_UPDATE * collide_t;
					}

					state->playing.dampen_calories_burned = state->playing.calories_burned;

					state->type      = StateType::game_over;
					state->game_over = {};
				}
				else
				{
					FOR_ELEMS(it, state->belt_offsets)
					{
						*it += state->dampen_belt_velocities[it_index] * SECONDS_PER_UPDATE;
					}

					state->playing.ralph_position    += state->playing.ralph_velocity * SECONDS_PER_UPDATE;
					state->playing.obstacle_position += obstacle_velocity * SECONDS_PER_UPDATE;

					if (state->playing.ralph_position.y - RALPH_HITBOX_DIMENSIONS.y / 2.0f <= 0.0f)
					{
						state->playing.ralph_position.y  = RALPH_HITBOX_DIMENSIONS.y / 2.0f;
						state->playing.calories_burned  += fabsf(state->dampen_belt_velocities[state->playing.ralph_belt_index]) * CALORIES_PER_METER_PER_SECOND * SECONDS_PER_UPDATE;
						loop_sprite(&state->ralph_running_sprite, SECONDS_PER_UPDATE);
					}

					if (state->playing.obstacle_position.x + state->playing.obstacle_hitbox.x / 2.0f < 0.0f)
					{
						state->playing.obstacle_belt_index = rng(&state->seed, 0, 3);
						state->playing.obstacle_hitbox     = { 0.6f, 0.5f, 0.2f };
						state->playing.obstacle_position   = { WINDOW_DIMENSIONS.x / PIXELS_PER_METER + state->playing.obstacle_hitbox.x / 2.0f, state->playing.obstacle_hitbox.y / 2.0f, -(state->playing.obstacle_belt_index + 0.5f) * BELT_HEIGHT };
					}

					state->playing.dampen_calories_burned = dampen(state->playing.dampen_calories_burned, state->playing.calories_burned, 16.0f, SECONDS_PER_UPDATE);
				}
			} break;

			case StateType::game_over:
			{
				if (state->input.accept && !state->prev_input.accept)
				{
					state->type       = StateType::title_menu;
					state->title_menu = {};

					FOR_ELEMS(it, state->title_menu.initial_belt_offsets)
					{
						*it = TITLE_MENU_OPTIONS_WIDTH + fmodf(state->belt_offsets[it_index], BELT_SPACING);
						state->belt_offsets[it_index] = *it;
					}
				}
				else
				{
					FOR_ELEMS(it, state->dampen_belt_velocities)
					{
						*it = dampen(*it, state->belt_velocities[it_index], 4.0f, SECONDS_PER_UPDATE);
					}

					FOR_ELEMS(it, state->belt_offsets)
					{
						*it += state->dampen_belt_velocities[it_index] * SECONDS_PER_UPDATE;
					}

					state->playing.ralph_position.x    += state->dampen_belt_velocities[state->playing.ralph_belt_index] * SECONDS_PER_UPDATE;
					state->playing.obstacle_position.x += state->dampen_belt_velocities[state->playing.obstacle_belt_index] * SECONDS_PER_UPDATE;

					age_sprite(&state->ralph_exploding_sprite, SECONDS_PER_UPDATE);
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
			draw_rect(program->renderer, { 0.0f, belt_index * BELT_HEIGHT * PIXELS_PER_METER }, { WINDOW_DIMENSIONS.x, BELT_HEIGHT * PIXELS_PER_METER });

			set_color(program->renderer, monochrome(0.4f));
			FOR_RANGE(scale_index, static_cast<i32>(WINDOW_DIMENSIONS.x / (BELT_SPACING * PIXELS_PER_METER)) + 2)
			{
				vf2 mid = { fmodf(state->belt_offsets[belt_index], BELT_SPACING) + (scale_index - 1.0f) * BELT_SPACING, (belt_index + 0.5f) * BELT_HEIGHT };
				draw_line(program->renderer, mid * PIXELS_PER_METER, (mid + vf2 { BELT_SPACING, -BELT_HEIGHT / 2.0f }) * PIXELS_PER_METER);
				draw_line(program->renderer, mid * PIXELS_PER_METER, (mid + vf2 { BELT_SPACING,  BELT_HEIGHT / 2.0f }) * PIXELS_PER_METER);
			}
		}

		set_color(program->renderer, { 1.0f, 1.0f, 1.0f, 1.0f });
		draw_line(program->renderer, { 0.0f, BELT_HEIGHT * PIXELS_PER_METER        }, { WINDOW_DIMENSIONS.x, BELT_HEIGHT * PIXELS_PER_METER        });
		draw_line(program->renderer, { 0.0f, BELT_HEIGHT * PIXELS_PER_METER * 2.0f }, { WINDOW_DIMENSIONS.x, BELT_HEIGHT * PIXELS_PER_METER * 2.0f });

		if (state->type == StateType::title_menu || state->type == StateType::playing)
		{
			draw_text
			(
				program->renderer,
				state->font,
				{ WINDOW_DIMENSIONS.x / 2.0f + state->belt_offsets[2] * PIXELS_PER_METER, 2.5f * BELT_HEIGHT * PIXELS_PER_METER - FC_GetBaseline(state->font) / 2.0f },
				FC_ALIGN_CENTER,
				1.0f,
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				"Sushi Ralph"
			);

			FOR_ELEMS(it, TITLE_MENU_OPTIONS)
			{
				draw_text
				(
					program->renderer,
					state->font,
					{ WINDOW_DIMENSIONS.x / 2.0f + (it_index * TITLE_MENU_OPTION_SPACING + state->belt_offsets[1]) * PIXELS_PER_METER, 1.4f * BELT_HEIGHT * PIXELS_PER_METER },
					FC_ALIGN_CENTER,
					0.7f,
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					"%s", *it
				);
			}
		}

		if (state->type == StateType::playing)
		{
			draw_sprite(program->renderer, &state->ralph_running_sprite, project(state->playing.ralph_position));
			draw_sprite(program->renderer, &state->sushi_sprite, project(state->playing.obstacle_position));
			draw_text(program->renderer, state->font, { WINDOW_DIMENSIONS.x / 2.0f, (3.5f * BELT_HEIGHT) * PIXELS_PER_METER }, FC_ALIGN_CENTER, 0.5f, { 1.0f, 1.0f, 1.0f, 1.0f }, "Calories burned : %f", state->playing.dampen_calories_burned);

			set_color(program->renderer, { 0.0f, 1.0f, 0.0f, 1.0f });
			draw_hitbox(program->renderer, state->playing.ralph_position, RALPH_HITBOX_DIMENSIONS);

			set_color(program->renderer, { 1.0f, 1.0f, 0.0f, 1.0f });
			draw_hitbox(program->renderer, state->playing.obstacle_position, state->playing.obstacle_hitbox);
		}
		else if (state->type == StateType::game_over)
		{
			draw_sprite(program->renderer, &state->ralph_exploding_sprite, project(state->playing.ralph_position));
			draw_sprite(program->renderer, &state->sushi_sprite, project(state->playing.obstacle_position));
			draw_text(program->renderer, state->font, WINDOW_DIMENSIONS / 2.0f, FC_ALIGN_CENTER, 1.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, "GAME OVER");
			draw_text
			(
				program->renderer,
				state->font,
				WINDOW_DIMENSIONS / 2.0f - vf2 { 0.0f, 45.0f },
				FC_ALIGN_CENTER, 0.5f, { 1.0f, 1.0f, 1.0f, 1.0f },
				"Calories burned : %f\nHighest calories burned : %f", state->playing.calories_burned, state->highest_calories_burned
			);

			set_color(program->renderer, { 0.0f, 1.0f, 0.0f, 1.0f });
			draw_hitbox(program->renderer, state->playing.ralph_position, RALPH_HITBOX_DIMENSIONS);

			set_color(program->renderer, { 1.0f, 1.0f, 0.0f, 1.0f });
			draw_hitbox(program->renderer, state->playing.obstacle_position, state->playing.obstacle_hitbox);
		}


		SDL_RenderPresent(program->renderer);

		state->prev_input = state->input;
	}
}
