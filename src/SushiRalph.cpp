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

internal inline bool32 colliding(vf3 corner_a, vf3 dimensions_a, vf3 corner_b, vf3 dimensions_b)
{
	return
		corner_a.x < corner_b.x + dimensions_b.x && corner_a.x + dimensions_a.x > corner_b.x &&
		corner_a.y < corner_b.y + dimensions_b.y && corner_a.y + dimensions_a.y > corner_b.y &&
		corner_a.z < corner_b.z + dimensions_b.z && corner_a.z + dimensions_a.z > corner_b.z;
}

internal bool32 colliding(f32* result, vf3 ray, vf3 corner_a, vf3 dimensions_a, vf3 corner_b, vf3 dimensions_b)
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

internal Sprite load_sprite(SDL_Renderer* renderer, strlit file_path, f32 scalar, vf2 origin, i32 frame_count = 1, f32 seconds_per_frame = 0.0f)
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
	sprite.origin              = origin;
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

internal Obstacle make_obstacle(State* state)
{
	Obstacle obstacle;
	obstacle.sprite_index = rng(&state->seed, 0, ARRAY_CAPACITY(state->obstacle_sprites));
	obstacle.belt_index   = rng(&state->seed, 0, 3);
	obstacle.position     = { WINDOW_DIMENSIONS.x / PIXELS_PER_METER + OBSTACLE_ASSETS[obstacle.sprite_index].hitbox.x + rng(&state->seed, 0.0f, 4.0f), OBSTACLE_ASSETS[obstacle.sprite_index].hitbox.y / 2.0f, -(obstacle.belt_index + 0.5f) * BELT_HEIGHT };

	// @TODO@ Optimize?
	FOR_ELEMS(it, state->playing.obstacles)
	{
		if (colliding(obstacle.position, OBSTACLE_ASSETS[obstacle.sprite_index].hitbox, it->position, OBSTACLE_ASSETS[it->sprite_index].hitbox))
		{
			obstacle.position.x = it->position.x + OBSTACLE_ASSETS[it->sprite_index].hitbox.x + OBSTACLE_ASSETS[obstacle.sprite_index].hitbox.x + 1.0f;
			it_index = -1;
		}
	}

	return obstacle;
}

extern "C" PROTOTYPE_INITIALIZE(initialize)
{
	State* state = reinterpret_cast<State*>(program->memory);

	*state = {};
	state->type                         = StateType::title_menu;
	state->title_menu.resetting_keytime = 1.0f;

	state->settings.master_volume = 1.0f;
	state->settings.music_volume  = 1.0f;
	state->settings.sfx_volume    = 1.0f;

	state->background_music_keytime_dampening = 4.0f;
}

extern "C" PROTOTYPE_BOOT_UP(boot_up)
{
	State* state = reinterpret_cast<State*>(program->memory);

	state->font = FC_CreateFont();
	FC_LoadFont(state->font, program->renderer, "C:/code/misc/fonts/Consolas.ttf", 64, { 255, 255, 255, 255 }, TTF_STYLE_NORMAL);

	state->ralph_running_sprite   = load_sprite(program->renderer, "W:/data/ralph_running.bmp"   , 0.6f, { 0.5f, 0.4f }, 4, 0.25f);
	state->ralph_exploding_sprite = load_sprite(program->renderer, "W:/data/ralph_exploding.bmp" , 0.6f, { 0.5f, 0.4f }, 4, 0.15f);
	state->shadow_sprite          = load_sprite(program->renderer, "W:/data/shadow.bmp", 0.2f, { 0.5f, 1.0f });

	FOR_ELEMS(asset, OBSTACLE_ASSETS)
	{
		state->obstacle_sprites[asset_index] = load_sprite(program->renderer, asset->file_path, asset->scalar, asset->origin);
	}

	state->background_music         = Mix_LoadWAV("W:/data/Giant Steps.wav");
	state->background_music_muffled = Mix_LoadWAV("W:/data/Giant Steps Muffled.wav");
	state->explosion_sfx            = Mix_LoadWAV("W:/data/explosion.wav");
	state->chomp_sfx                = Mix_LoadWAV("W:/data/chomp.wav");

	FILE* save_data;
	errno_t save_data_error = fopen_s(&save_data, SAVE_DATA_FILE_PATH, "rb");

	if (!save_data_error)
	{
		fread(&state->highest_calories_burned, 1, sizeof(state->highest_calories_burned), save_data);
		fread(&state->settings.master_volume , 1, sizeof(state->settings.master_volume) , save_data);
		fread(&state->settings.music_volume  , 1, sizeof(state->settings.music_volume)  , save_data);
		fread(&state->settings.sfx_volume    , 1, sizeof(state->settings.sfx_volume)    , save_data);
		fclose(save_data);
	}
	else
	{
		DEBUG_printf("No save file found at '%s'.\n", SAVE_DATA_FILE_PATH);
		state->highest_calories_burned = 0.0f;
	}

	Mix_PlayChannel(+AudioChannel::background_music        , state->background_music        , -1);
	Mix_PlayChannel(+AudioChannel::background_music_muffled, state->background_music_muffled, -1);
	Mix_Volume(+AudioChannel::background_music        ,              0);
	Mix_Volume(+AudioChannel::background_music_muffled, MIX_MAX_VOLUME);
}

extern "C" PROTOTYPE_BOOT_DOWN(boot_down)
{
	State* state = reinterpret_cast<State*>(program->memory);

	Mix_FreeChunk(state->chomp_sfx);
	Mix_FreeChunk(state->explosion_sfx);
	Mix_FreeChunk(state->background_music_muffled);
	Mix_FreeChunk(state->background_music);

	FILE* save_data;
	errno_t save_data_error = fopen_s(&save_data, SAVE_DATA_FILE_PATH, "wb");

	if (save_data_error == 0)
	{
		fwrite(&state->highest_calories_burned, 1, sizeof(state->highest_calories_burned), save_data);
		fwrite(&state->settings.master_volume , 1, sizeof(state->settings.master_volume) , save_data);
		fwrite(&state->settings.music_volume  , 1, sizeof(state->settings.music_volume)  , save_data);
		fwrite(&state->settings.sfx_volume    , 1, sizeof(state->settings.sfx_volume)    , save_data);
		fclose(save_data);
	}
	else
	{
		DEBUG_printf("Could not save at '%s'.\n", SAVE_DATA_FILE_PATH);
	}

	FC_FreeFont(state->font);
	SDL_DestroyTexture(state->ralph_running_sprite.texture);
	SDL_DestroyTexture(state->ralph_exploding_sprite.texture);
	SDL_DestroyTexture(state->shadow_sprite.texture);

	FOR_ELEMS(it, state->obstacle_sprites)
	{
		SDL_DestroyTexture(it->texture);
	}
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

		state->dampen_background_music_keytime = dampen(state->dampen_background_music_keytime, state->background_music_keytime, state->background_music_keytime_dampening, SECONDS_PER_UPDATE);
		Mix_Volume(+AudioChannel::background_music        , static_cast<i32>(        state->dampen_background_music_keytime  * state->settings.music_volume * state->settings.master_volume * MIX_MAX_VOLUME));
		Mix_Volume(+AudioChannel::background_music_muffled, static_cast<i32>((1.0f - state->dampen_background_music_keytime) * state->settings.music_volume * state->settings.master_volume * MIX_MAX_VOLUME));
		Mix_VolumeChunk(state->explosion_sfx, static_cast<i32>(state->settings.sfx_volume * state->settings.master_volume * MIX_MAX_VOLUME));
		Mix_VolumeChunk(state->chomp_sfx    , static_cast<i32>(state->settings.sfx_volume * state->settings.master_volume * MIX_MAX_VOLUME));

		//
		// Update.
		//

		switch (state->type)
		{
			case StateType::title_menu:
			{
				if (state->title_menu.resetting_keytime < 1.0f)
				{
					state->title_menu.resetting_keytime += SECONDS_PER_UPDATE / 1.0f;

					if (state->title_menu.resetting_keytime >= 1.0f)
					{
						state->title_menu.resetting_keytime = 1.0f;

						FOR_ELEMS(it, state->belt_offsets)
						{
							if (it_index == 1)
							{
								*it = -TITLE_MENU_OPTION_SPACING * state->title_menu.option_index;
							}
							else
							{
								*it = 0.0f;
							}
						}
					}
					else
					{
						FOR_ELEMS(it, state->belt_offsets)
						{
							if (it_index == 1)
							{
								*it = lerp(state->title_menu.initial_belt_offsets[it_index], -TITLE_MENU_OPTION_SPACING * state->title_menu.option_index, square(ease_in(state->title_menu.resetting_keytime)));
							}
							else
							{
								*it = lerp(state->title_menu.initial_belt_offsets[it_index], 0.0f, square(ease_in(state->title_menu.resetting_keytime)));
							}
						}
					}
				}

				if (state->title_menu.resetting_keytime == 1.0f)
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

					state->belt_velocities[1] = (-state->belt_offsets[1] - state->title_menu.option_index * TITLE_MENU_OPTION_SPACING) * 16.0f;

					FOR_ELEMS(it, state->dampen_belt_velocities)
					{
						*it = dampen(*it, state->belt_velocities[it_index], 24.0f, SECONDS_PER_UPDATE);
					}

					FOR_ELEMS(it, state->belt_offsets)
					{
						*it += state->dampen_belt_velocities[it_index] * SECONDS_PER_UPDATE;
					}

					if (state->input.accept && !state->prev_input.accept)
					{
						switch (state->title_menu.option_index)
						{
							case 0:
							{
								FOR_ELEMS(it, state->belt_velocities)
								{
									*it = rng(&state->seed, -BELT_MIN_SPEED, -BELT_MAX_SPEED);
								}
								FOR_ELEMS(it, state->dampen_belt_velocities)
								{
									*it = 0.0f;
								}

								state->type                          = StateType::playing;
								state->playing                       = {};
								state->playing.ralph_belt_index      = 1;
								state->playing.ralph_position        = { -3.0f, RALPH_HITBOX_DIMENSIONS.y / 2.0f, -1.5f * BELT_HEIGHT };

								FOR_ELEMS(it, state->playing.obstacles)
								{
									*it = make_obstacle(state);
								}
							} break;

							case 1:
							{
								FOR_ELEMS(it, state->belt_velocities)
								{
									*it = 0.0f;
									state->dampen_belt_velocities[it_index] = 0.0f;
								}

								state->type                  = StateType::settings;
								state->settings.showing      = true;
								state->settings.show_keytime = 0.0f;

								FOR_ELEMS(it, state->settings.initial_belt_offsets)
								{
									*it = state->belt_offsets[it_index];
								}

								state->settings.option_index  = 1;
							} break;

							case 2:
							{
								state->type            = StateType::credits;
								state->credits         = {};
								state->credits.showing = true;

								FOR_ELEMS(it, state->credits.initial_belt_offsets)
								{
									*it = state->belt_offsets[it_index];
								}
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

			case StateType::settings:
			{
				if (state->settings.showing)
				{
					if (state->settings.show_keytime < 1.0f)
					{
						state->settings.show_keytime += SECONDS_PER_UPDATE / 1.0f;

						if (state->settings.show_keytime >= 1.0f)
						{
							state->settings.show_keytime = 1.0f;
							state->belt_offsets[1] = -SETTINGS_OPTIONS_OFFSET - state->settings.option_index * SETTINGS_OPTION_SPACING;
						}
						else
						{
							state->belt_offsets[1] = lerp(state->settings.initial_belt_offsets[1], -SETTINGS_OPTIONS_OFFSET - state->settings.option_index * SETTINGS_OPTION_SPACING, square(ease_in(state->settings.show_keytime)));
						}
					}

					if (state->settings.show_keytime == 1.0f)
					{
						if (state->settings.changing_option)
						{
							if (state->input.accept && !state->prev_input.accept)
							{
								state->settings.changing_option = false;
							}
							else
							{
								f32* option = 0;

								switch (state->settings.option_index)
								{
									case 1: option = &state->settings.master_volume; break;
									case 2: option = &state->settings.music_volume;  break;
									case 3: option = &state->settings.sfx_volume;    break;
								}

								if (state->input.left)
								{
									*option -= SETTINGS_VOLUME_SLIDE_SPEED * SECONDS_PER_UPDATE;
									*option = CLAMP(*option, 0.0f, 1.0f);
								}
								if (state->input.right)
								{
									*option += SETTINGS_VOLUME_SLIDE_SPEED * SECONDS_PER_UPDATE;
									*option = CLAMP(*option, 0.0f, 1.0f);
								}

								state->belt_velocities[0] = (-SETTINGS_VOLUME_SLIDE_OFFSET - *option * SETTINGS_VOLUME_SLIDE_WIDTH - state->belt_offsets[0]) * 4.0f;
							}
						}
						else
						{
							// @TODO@ Make holding down work nicely.
							if (state->input.left && !state->prev_input.left && state->settings.option_index > 0)
							{
								--state->settings.option_index;
							}
							if (state->input.right && !state->prev_input.right && state->settings.option_index < ARRAY_CAPACITY(SETTINGS_OPTIONS) - 1)
							{
								++state->settings.option_index;
							}

							if (state->input.accept && !state->prev_input.accept)
							{
								if (state->settings.option_index == 0)
								{
									state->settings.showing                 = false;
									state->settings.initial_belt_offsets[0] = state->belt_offsets[0];
								}
								else
								{
									state->settings.changing_option = true;
								}
							}

							state->belt_velocities[0] = (0.0f - state->belt_offsets[0]) * 4.0f;
						}

						state->belt_velocities[1] = (-SETTINGS_OPTIONS_OFFSET - state->belt_offsets[1] - state->settings.option_index * SETTINGS_OPTION_SPACING) * 16.0f;

						FOR_ELEMS(it, state->dampen_belt_velocities)
						{
							*it = dampen(*it, state->belt_velocities[it_index], 24.0f, SECONDS_PER_UPDATE);
						}

						FOR_ELEMS(it, state->belt_offsets)
						{
							*it += state->dampen_belt_velocities[it_index] * SECONDS_PER_UPDATE;
						}
					}
				}
				else if (state->settings.show_keytime > 0.0f)
				{
					state->settings.show_keytime -= SECONDS_PER_UPDATE / 1.0f;

					if (state->settings.show_keytime <= 0.0f)
					{
						state->settings.show_keytime = 0.0f;

						FOR_ELEMS(it, state->belt_velocities)
						{
							*it = 0.0f;
							state->dampen_belt_velocities[it_index] = 0.0f;
						}

						state->belt_offsets[0] = 0.0f;

						state->type                         = StateType::title_menu;
						state->title_menu.resetting_keytime = 1.0f;
					}
					else
					{
						state->belt_offsets[1] = lerp(-SETTINGS_OPTIONS_OFFSET - state->settings.option_index * SETTINGS_OPTION_SPACING, state->settings.initial_belt_offsets[1], square(ease_in(1.0f - state->settings.show_keytime)));
						state->belt_offsets[0] = lerp(state->settings.initial_belt_offsets[0], 0.0f, square(ease_in(1.0f - state->settings.show_keytime)));
					}
				}

			} break;

			case StateType::credits:
			{
				if (state->credits.showing)
				{
					if (state->credits.keytime < 1.0f)
					{
						state->credits.keytime += SECONDS_PER_UPDATE / 1.0f;

						if (state->credits.keytime >= 1.0f)
						{
							state->credits.keytime = 1.0f;
							state->belt_offsets[0] = -CREDITS_OFFSET;
						}
						else
						{
							state->belt_offsets[0] = lerp(state->credits.initial_belt_offsets[0], -CREDITS_OFFSET, square(ease_in(state->credits.keytime)));
						}
					}

					if (state->credits.keytime == 1.0f)
					{
						if (state->input.accept)
						{
							state->credits.showing                 = false;
							state->credits.initial_belt_offsets[0] = state->belt_offsets[0];

							state->belt_velocities[0]        = 0.0f;
							state->dampen_belt_velocities[0] = 0.0f;
						}
						else
						{
							if (state->input.left && !state->prev_input.left && state->credits.credit_index > 0)
							{
								--state->credits.credit_index;
							}
							if (state->input.right && !state->prev_input.right && state->credits.credit_index < ARRAY_CAPACITY(CREDITS) - 1)
							{
								++state->credits.credit_index;
							}

							state->belt_velocities[0] = (-state->belt_offsets[0] - state->credits.credit_index * CREDIT_SPACING - CREDITS_OFFSET) * 8.0f;
						}
					}

					state->belt_velocities[1] = (-state->belt_offsets[1] - state->title_menu.option_index * TITLE_MENU_OPTION_SPACING) * 16.0f;

					FOR_ELEMS(it, state->dampen_belt_velocities)
					{
						*it = dampen(*it, state->belt_velocities[it_index], 24.0f, SECONDS_PER_UPDATE);
					}

					FOR_ELEMS(it, state->belt_offsets)
					{
						*it += state->dampen_belt_velocities[it_index] * SECONDS_PER_UPDATE;
					}
				}
				else
				{
					if (state->credits.keytime > 0.0f)
					{
						state->credits.keytime -= SECONDS_PER_UPDATE / 1.0f;

						if (state->credits.keytime <= 0.0f)
						{
							state->credits.keytime = 0.0f;
							state->belt_offsets[0] = 0.0f;
						}
						else
						{
							state->belt_offsets[0] = lerp(state->credits.initial_belt_offsets[0], 0.0f, square(ease_in(1.0f - state->credits.keytime)));
						}
					}

					if (state->credits.keytime == 0.0f)
					{
						state->type = StateType::title_menu;
					}
				}
			} break;

			case StateType::playing:
			{
				if (state->playing.belt_velocity_update_keytime < 1.0f)
				{
					state->playing.belt_velocity_update_keytime += SECONDS_PER_UPDATE / 4.0f;

					if (state->playing.belt_velocity_update_keytime >= 1.0f)
					{
						state->playing.belt_velocity_update_keytime = 1.0f;

						FOR_ELEMS(it, state->belt_velocities)
						{
							*it = rng(&state->seed, -BELT_MIN_SPEED, -BELT_MAX_SPEED);
						}

						state->playing.belt_velocity_update_keytime = 0.0f;
					}
				}

				FOR_ELEMS(it, state->dampen_belt_velocities)
				{
					*it = dampen(*it, state->belt_velocities[it_index], 1.0f, SECONDS_PER_UPDATE);
				}

				state->playing.target_ralph_velocity_x = (RALPH_X - state->playing.ralph_position.x) * 4.0f;
				state->playing.ralph_velocity.x        = dampen(state->playing.ralph_velocity.x, state->playing.target_ralph_velocity_x, 2.0f, SECONDS_PER_UPDATE);

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

					state->ralph_running_sprite.seconds_per_frame = sigmoid(state->playing.ralph_velocity.x, -0.75f);

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

				bool32 collided                = false;
				f32    collide_t               = INFINITY;
				i32    collided_obstacle_index = -1;

				FOR_ELEMS(it, state->playing.obstacles)
				{
					f32 it_collide_t;
					if
					(
						colliding
						(
							&it_collide_t,
							state->playing.ralph_velocity * SECONDS_PER_UPDATE,
							state->playing.ralph_position - RALPH_HITBOX_DIMENSIONS / 2.0f,
							RALPH_HITBOX_DIMENSIONS,
							it->position - OBSTACLE_ASSETS[it->sprite_index].hitbox / 2.0f,
							OBSTACLE_ASSETS[it->sprite_index].hitbox
						)
						&& it_collide_t < collide_t // @TODO@ Should be based on distance.
					)
					{
						collided                = true;
						collide_t               = it_collide_t;
						collided_obstacle_index = it_index;
					}
				}

				if (collided)
				{
					state->playing.calories_burned -= state->playing.peak_calories_burned * 0.25f + state->playing.time / 4.0f + 10.0f;

					if (state->playing.calories_burned >= 0.0f)
					{
						Mix_PlayChannel(-1, state->chomp_sfx, 0);

						state->playing.obstacles[collided_obstacle_index] = make_obstacle(state);
					}
				}

				if (state->playing.calories_burned < 0.0f)
				{
					Mix_PlayChannel(-1, state->explosion_sfx, 0);

					FOR_ELEMS(it, state->belt_offsets)
					{
						*it += state->dampen_belt_velocities[it_index] * SECONDS_PER_UPDATE * collide_t;
					}

					FOR_ELEMS(it, state->playing.obstacles)
					{
						it->position += vf3 { state->dampen_belt_velocities[it->belt_index], 0.0f, 0.0f } * SECONDS_PER_UPDATE * collide_t;
					}

					FOR_ELEMS(it, state->belt_velocities)
					{
						*it = 0.0f;
					}

					state->highest_calories_burned = MAXIMUM(state->highest_calories_burned, state->playing.calories_burned);

					state->ralph_exploding_sprite.frame_index = 0;

					state->playing.ralph_position += (state->playing.ralph_velocity + vf3 { state->dampen_belt_velocities[state->playing.ralph_belt_index], 0.0f, 0.0f }) * SECONDS_PER_UPDATE * collide_t;

					if (state->playing.ralph_position.y - RALPH_HITBOX_DIMENSIONS.y / 2.0f <= 0.0f)
					{
						state->playing.ralph_position.y  = RALPH_HITBOX_DIMENSIONS.y / 2.0f;
						state->playing.calories_burned  += fabsf(state->dampen_belt_velocities[state->playing.ralph_belt_index] * SECONDS_PER_UPDATE * collide_t) * CALORIES_PER_METER;
					}

					state->background_music_keytime = 0.0f;

					state->playing.time += SECONDS_PER_UPDATE * collide_t;

					state->type      = StateType::game_over;
					state->game_over = {};

					FOR_ELEMS(it, state->game_over.initial_belt_offsets)
					{
						*it = state->belt_offsets[it_index];
					}

					state->game_over.stat_belt_index = rng(&state->seed, 0, ARRAY_CAPACITY(state->belt_offsets));

					if (state->playing.ralph_belt_index == state->game_over.stat_belt_index)
					{
						state->game_over.stat_belt_index = (state->game_over.stat_belt_index + 1) % ARRAY_CAPACITY(state->belt_offsets);
					}
				}
				else
				{
					FOR_ELEMS(it, state->belt_offsets)
					{
						*it += state->dampen_belt_velocities[it_index] * SECONDS_PER_UPDATE;
					}

					FOR_ELEMS(it, state->playing.obstacles)
					{
						it->position += vf3 { state->dampen_belt_velocities[it->belt_index], 0.0f, 0.0f } * SECONDS_PER_UPDATE;

						if (it->position.x + OBSTACLE_ASSETS[it->sprite_index].hitbox.x / 2.0f < 0.0f)
						{
							*it = make_obstacle(state);
						}
					}

					state->playing.ralph_position += (state->playing.ralph_velocity + vf3 { state->dampen_belt_velocities[state->playing.ralph_belt_index], 0.0f, 0.0f }) * SECONDS_PER_UPDATE;

					if (state->playing.ralph_position.y - RALPH_HITBOX_DIMENSIONS.y / 2.0f <= 0.0f)
					{
						state->playing.ralph_position.y  = RALPH_HITBOX_DIMENSIONS.y / 2.0f;
						state->playing.calories_burned  += fabsf(state->dampen_belt_velocities[state->playing.ralph_belt_index] * SECONDS_PER_UPDATE) * CALORIES_PER_METER;
						loop_sprite(&state->ralph_running_sprite, SECONDS_PER_UPDATE);
						state->background_music_keytime = sigmoid(state->belt_velocities[state->playing.ralph_belt_index] + 3.0f, -1.0f);
					}
					else
					{
						state->background_music_keytime = sigmoid(0.1f, -1.0f);
					}

					state->playing.time += SECONDS_PER_UPDATE;
				}

				if (state->playing.calories_burned > state->playing.peak_calories_burned)
				{
					state->playing.peak_calories_burned = state->playing.calories_burned;

					if (state->playing.peak_calories_burned > state->highest_calories_burned)
					{
						state->highest_calories_burned = state->playing.peak_calories_burned;
					}
				}
			} break;

			case StateType::game_over:
			{
				if (state->game_over.exiting)
				{
					if (state->game_over.keytime < 1.0f)
					{
						state->game_over.keytime += SECONDS_PER_UPDATE / 1.5f;

						if (state->game_over.keytime >= 1.0f)
						{
							state->game_over.keytime = 1.0f;
						}
					}

					FOR_ELEMS(it, state->dampen_belt_velocities)
					{
						*it = dampen(*it, state->belt_velocities[it_index], 4.0f, SECONDS_PER_UPDATE);
					}

					FOR_ELEMS(it, state->belt_offsets)
					{
						*it += state->dampen_belt_velocities[it_index] * SECONDS_PER_UPDATE;
					}

					state->playing.ralph_position.x += state->dampen_belt_velocities[state->playing.ralph_belt_index] * SECONDS_PER_UPDATE;

					FOR_ELEMS(it, state->playing.obstacles)
					{
						it->position.x += state->dampen_belt_velocities[it->belt_index] * SECONDS_PER_UPDATE;
					}

					if (state->game_over.keytime == 1.0f)
					{
						FOR_ELEMS(it, state->belt_velocities)
						{
							*it = 0.0f;
						}

						FOR_ELEMS(it, state->dampen_belt_velocities)
						{
							*it = 0.0f;
						}

						state->type       = StateType::title_menu;
						state->title_menu = {};

						FOR_ELEMS(it, state->title_menu.initial_belt_offsets)
						{
							*it = TITLE_MENU_OPTIONS_WIDTH + fmodf(state->belt_offsets[it_index], BELT_SPACING) + rng(&state->seed, 0, 16) * BELT_SPACING;
							state->belt_offsets[it_index] = *it;
						}
					}
				}
				else
				{
					if (state->game_over.keytime < 1.0f)
					{
						state->game_over.keytime += SECONDS_PER_UPDATE / 1.0f;

						if (state->game_over.keytime >= 1.0f)
						{
							state->game_over.keytime = 1.0f;
						}
					}

					state->belt_velocities[state->game_over.stat_belt_index] = (state->game_over.initial_belt_offsets[state->game_over.stat_belt_index] - GAME_OVER_STATS_OFFSET - state->belt_offsets[state->game_over.stat_belt_index]) * 1.5f;

					FOR_ELEMS(it, state->dampen_belt_velocities)
					{
						*it = dampen(*it, state->belt_velocities[it_index], 4.0f, SECONDS_PER_UPDATE);
					}

					FOR_ELEMS(it, state->belt_offsets)
					{
						*it += state->dampen_belt_velocities[it_index] * SECONDS_PER_UPDATE;
					}

					state->playing.ralph_position.x += state->dampen_belt_velocities[state->playing.ralph_belt_index] * SECONDS_PER_UPDATE;

					FOR_ELEMS(it, state->playing.obstacles)
					{
						it->position.x += state->dampen_belt_velocities[it->belt_index] * SECONDS_PER_UPDATE;
					}

					age_sprite(&state->ralph_exploding_sprite, SECONDS_PER_UPDATE);

					if (state->game_over.keytime == 1.0f)
					{
						if (state->input.accept && !state->prev_input.accept)
						{
							state->game_over.exiting = true;
							state->game_over.keytime = 0.0f;

							FOR_ELEMS(it, state->belt_velocities)
							{
								*it = -10.0f;
							}
						}
					}
				}
			} break;
		}

		//
		// Render.
		//

		set_color(program->renderer, { 0.1f, 0.2f, 0.3f, 1.0f });
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

		if (state->type == StateType::title_menu || state->type == StateType::playing || state->type == StateType::settings || state->type == StateType::credits)
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

			if (state->type == StateType::settings)
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

				FOR_ELEMS(it, SETTINGS_OPTIONS)
				{
					draw_text
					(
						program->renderer,
						state->font,
						{ WINDOW_DIMENSIONS.x / 2.0f + (it_index * SETTINGS_OPTION_SPACING + state->belt_offsets[1] + SETTINGS_OPTIONS_OFFSET) * PIXELS_PER_METER, 1.4f * BELT_HEIGHT * PIXELS_PER_METER },
						FC_ALIGN_CENTER,
						0.7f,
						{ 1.0f, 1.0f, 1.0f, 1.0f },
						"%s", *it
					);
				}

				draw_text
				(
					program->renderer,
					state->font,
					{ WINDOW_DIMENSIONS.x / 2.0f + (state->belt_offsets[0] + SETTINGS_VOLUME_SLIDE_OFFSET) * PIXELS_PER_METER, 0.5f * BELT_HEIGHT * PIXELS_PER_METER - FC_GetBaseline(state->font) / 2.0f },
					FC_ALIGN_CENTER,
					1.0f,
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					"MIN"
				);

				draw_text
				(
					program->renderer,
					state->font,
					{ WINDOW_DIMENSIONS.x / 2.0f + (state->belt_offsets[0] + SETTINGS_VOLUME_SLIDE_OFFSET + SETTINGS_VOLUME_SLIDE_WIDTH) * PIXELS_PER_METER, 0.5f * BELT_HEIGHT * PIXELS_PER_METER - FC_GetBaseline(state->font) / 2.0f },
					FC_ALIGN_CENTER,
					1.0f,
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					"MAX"
				);
			}
			else if (state->type == StateType::credits)
			{
				FOR_ELEMS(it, CREDITS)
				{
					draw_text
					(
						program->renderer,
						state->font,
						{ WINDOW_DIMENSIONS.x / 2.0f + (state->belt_offsets[0] + CREDITS_OFFSET + it_index * CREDIT_SPACING) * PIXELS_PER_METER, 0.5f * BELT_HEIGHT * PIXELS_PER_METER - FC_GetBaseline(state->font) * 0.25f },
						FC_ALIGN_CENTER,
						0.5f,
						{ 1.0f, 1.0f, 1.0f, 1.0f },
						*it
					);
				}
			}
		}

		if (state->type == StateType::playing || state->type == StateType::game_over)
		{
			FOR_RANGE_REV(i, ARRAY_CAPACITY(state->belt_offsets))
			{
				FOR_ELEMS(it, state->playing.obstacles)
				{
					if (it->belt_index == i)
					{
						draw_sprite(program->renderer, &state->obstacle_sprites[it->sprite_index], project(it->position));
						set_color(program->renderer, { 1.0f, 1.0f, 0.0f, 1.0f });
					}
				}
			}

			set_color(program->renderer, { 1.0f, 0.0f, 0.0f, 1.0f });

			if (state->type == StateType::playing)
			{
				SDL_SetTextureAlphaMod(state->shadow_sprite.texture, static_cast<u8>(255.0f * CLAMP(1.0f - state->playing.ralph_position.y / 4.0f, 0.0f, 1.0f)));
				draw_sprite(program->renderer, &state->shadow_sprite, project({ state->playing.ralph_position.x, 0.0f, state->playing.ralph_position.z }));
				draw_sprite(program->renderer, &state->ralph_running_sprite, project(state->playing.ralph_position));
				draw_text
				(
					program->renderer,
					state->font,
					project({ state->playing.ralph_position.x, 0.0f, state->playing.ralph_position.z + BELT_HEIGHT * 0.45f }),
					FC_ALIGN_CENTER,
					0.3f,
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					"Calories burned : %.2f", state->playing.calories_burned
				);
			}
			else
			{
				draw_sprite(program->renderer, &state->ralph_exploding_sprite, project(state->playing.ralph_position));
				draw_text
				(
					program->renderer,
					state->font,
					{ WINDOW_DIMENSIONS.x / 2.0f + (state->belt_offsets[state->game_over.stat_belt_index] - state->game_over.initial_belt_offsets[state->game_over.stat_belt_index] + GAME_OVER_STATS_OFFSET) * PIXELS_PER_METER, (state->game_over.stat_belt_index + 0.5f) * BELT_HEIGHT * PIXELS_PER_METER + FC_GetBaseline(state->font) * 0.2f },
					FC_ALIGN_CENTER,
					0.5f,
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					"Peak calories burned : %.2f\nRecord highest calories burned : %.2f", state->playing.peak_calories_burned, state->highest_calories_burned
				);
			}
		}


		SDL_RenderPresent(program->renderer);

		state->prev_input = state->input;
	}
}
