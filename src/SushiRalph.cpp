#include "SushiRalph.h"
#include "renderer.cpp"

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

		if (state->input.down)
		{
			state->offset -= 1.0f * SECONDS_PER_UPDATE;
		}
		if (state->input.up)
		{
			state->offset += 1.0f * SECONDS_PER_UPDATE;
		}

		state->offset = state->offset - static_cast<i32>(state->offset); // @TODO@ Make this where it is in the interval [0, 1)?

		//
		// Render.
		//

		set_color(program->renderer, {});
		SDL_RenderClear(program->renderer);

		draw_text(program->renderer, state->font, WINDOW_DIMENSIONS / 2.0f, FC_ALIGN_CENTER, 1.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, "Sushi Ralph");

		set_color(program->renderer, { 1.0f, 1.0f, 1.0f, 1.0f });
		draw_line(program->renderer, { 0.0f, WINDOW_DIMENSIONS.y        / 3.0f }, { WINDOW_DIMENSIONS.x, WINDOW_DIMENSIONS.y        / 3.0f });
		draw_line(program->renderer, { 0.0f, WINDOW_DIMENSIONS.y * 2.0f / 3.0f }, { WINDOW_DIMENSIONS.x, WINDOW_DIMENSIONS.y * 2.0f / 3.0f });

		FOR_RANGE(i, 0, static_cast<i32>(WINDOW_DIMENSIONS.x / BELT_SPACING) + 2)
		{
			draw_line(program->renderer, { (state->offset + i) * BELT_SPACING, WINDOW_DIMENSIONS.y * 2.0f / 3.0f }, { (state->offset + i - 1.0f) * BELT_SPACING, WINDOW_DIMENSIONS.y / 2.0f });
			draw_line(program->renderer, { (state->offset + i) * BELT_SPACING, WINDOW_DIMENSIONS.y        / 3.0f }, { (state->offset + i - 1.0f) * BELT_SPACING, WINDOW_DIMENSIONS.y / 2.0f });
		}

		SDL_RenderPresent(program->renderer);

		state->prev_input = state->input;
	}
}
