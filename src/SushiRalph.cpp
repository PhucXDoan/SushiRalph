#include "SushiRalph.h"
#include "render.cpp"

internal inline f32 lerp(f32 a, f32 b, f32 t) { return a * (1.0f - t) + b * t; }
internal inline vf3 lerp(vf3 a, vf3 b, f32 t) { return a * (1.0f - t) + b * t; }
internal inline vf4 lerp(vf4 a, vf4 b, f32 t) { return a * (1.0f - t) + b * t; }

internal inline f32 dampen(f32 a, f32 b, f32 k, f32 dt) { return lerp(a, b, 1.0f - expf(-k * dt)); }
internal inline vf3 dampen(vf3 a, vf3 b, f32 k, f32 dt) { return lerp(a, b, 1.0f - expf(-k * dt)); }
internal inline vf4 dampen(vf4 a, vf4 b, f32 k, f32 dt) { return lerp(a, b, 1.0f - expf(-k * dt)); }

extern "C" PROTOTYPE_INITIALIZE(initialize)
{
	State* state = reinterpret_cast<State*>(program->memory);

	state = {};
}

extern "C" PROTOTYPE_BOOT_UP(boot_up)
{
	State* state = reinterpret_cast<State*>(program->memory);

	state->font = FC_CreateFont();
	FC_LoadFont(state->font, program->renderer, "C:/code/misc/fonts/Consolas.ttf", 64, { 255, 255, 255, 255 }, TTF_STYLE_NORMAL);
}

extern "C" PROTOTYPE_BOOT_DOWN(boot_down)
{
	State* state = reinterpret_cast<State*>(program->memory);

	FC_FreeFont(state->font);
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

		// @TODO@ Make holding down work nicely.
		if (state->input.left && !state->prev_input.left && state->title_menu_option_index > 0)
		{
			--state->title_menu_option_index;
		}
		if (state->input.right && !state->prev_input.right && state->title_menu_option_index < ARRAY_CAPACITY(TITLE_MENU_OPTIONS) - 1)
		{
			++state->title_menu_option_index;
		}

		if (state->input.accept && !state->prev_input.accept)
		{
			switch (state->title_menu_option_index)
			{
				case 0:
				{
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

		state->belt_offsets[1] = dampen(state->belt_offsets[1], -state->title_menu_option_index * TITLE_MENU_OPTION_SPACING, 16.0f, SECONDS_PER_UPDATE);

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

		draw_text(program->renderer, state->font, WINDOW_DIMENSIONS / 2.0f + vf2 { 0.0f, BELT_HEIGHT - FC_GetBaseline(state->font) / 2.0f }, FC_ALIGN_CENTER, 1.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, "Sushi Ralph");

		FOR_ELEMS(it, TITLE_MENU_OPTIONS)
		{
			draw_text(program->renderer, state->font, WINDOW_DIMENSIONS / 2.0f + vf2 { it_index * TITLE_MENU_OPTION_SPACING + state->belt_offsets[1], -10.0f }, FC_ALIGN_CENTER, 0.7f, { 1.0f, 1.0f, 1.0f, 1.0f }, "%s", *it);
		}

		SDL_RenderPresent(program->renderer);

		state->prev_input = state->input;
	}
}
