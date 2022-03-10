internal vf4 monochrome(f32 k, f32 a = 1.0f)
{
	return { k, k, k, a };
}

internal void set_color(SDL_Renderer* renderer, vf4 rgba)
{
	rgba *= 255.0f;
	SDL_SetRenderDrawColor(renderer, static_cast<u8>(rgba.r), static_cast<u8>(rgba.g), static_cast<u8>(rgba.b), static_cast<u8>(rgba.a));
}

internal vf2 project(vf3 p)
{
	return { p.x, -p.z + p.y * 0.5f };
}

internal inline void draw_line(SDL_Renderer* renderer, vf2 start, vf2 end)
{
	SDL_RenderDrawLine(renderer, static_cast<i32>(start.x), static_cast<i32>(WINDOW_DIMENSIONS.y - 1.0f - start.y), static_cast<i32>(end.x), static_cast<i32>(WINDOW_DIMENSIONS.y - 1.0f - end.y));
}

internal void draw_rect(SDL_Renderer* renderer, vf2 bottom_left, vf2 dimensions)
{
	SDL_Rect rect = { static_cast<i32>(bottom_left.x), static_cast<i32>(WINDOW_DIMENSIONS.y - 1.0f - bottom_left.y - dimensions.y), static_cast<i32>(dimensions.x), static_cast<i32>(dimensions.y) };
	SDL_RenderFillRect(renderer, &rect);
}

template <typename... ARGUMENTS>
internal void draw_text(SDL_Renderer* renderer, FC_Font* font, vf2 coordinates, FC_AlignEnum alignment, f32 scalar, vf4 rgba, strlit fstr, ARGUMENTS... parameters)
{
	rgba *= 256.0f;
	FC_DrawEffect
	(
		font,
		renderer,
		coordinates.x,
		WINDOW_DIMENSIONS.y - 1.0f - coordinates.y - FC_GetBaseline(font) * scalar,
		FC_MakeEffect
		(
			alignment,
			FC_MakeScale(scalar, scalar),
			FC_MakeColor(static_cast<u8>(CLAMP(rgba.r, 0.0f, 255.0f)), static_cast<u8>(CLAMP(rgba.g, 0.0f, 255.0f)), static_cast<u8>(CLAMP(rgba.b, 0.0f, 255.0f)), static_cast<u8>(CLAMP(rgba.a, 0.0f, 255.0f)))
		),
		fstr,
		parameters...
	);
}

internal void draw_sprite(SDL_Renderer* renderer, Sprite* sprite, vf2 bottom_left)
{
	if (IN_RANGE(sprite->frame_index, 0, sprite->frame_count))
	{
		SDL_Rect src = { sprite->frame_index * sprite->width_pixels, 0, sprite->width_pixels, sprite->height_pixels };
		SDL_Rect dst =
			{
				static_cast<i32>(bottom_left.x + sprite->width_pixels * sprite->scalar * (sprite->offset.x - 1.0f)),
				static_cast<i32>(WINDOW_DIMENSIONS.y - 1.0f + sprite->height_pixels * sprite->scalar * (sprite->offset.y - 1.0f) - bottom_left.y),
				static_cast<i32>(sprite->width_pixels  * sprite->scalar),
				static_cast<i32>(sprite->height_pixels * sprite->scalar)
			};

		SDL_RenderCopy(renderer, sprite->texture, &src, &dst);
	}
}

internal void draw_crosshair(SDL_Renderer* renderer, vf2 position, f32 length)
{
	draw_line(renderer, position - vf2 { length,   0.0f }, position + vf2 { length,  0.0f });
	draw_line(renderer, position - vf2 {   0.0f, length }, position + vf2 {  0.0f, length });
}

internal void draw_hitbox(SDL_Renderer* renderer, vf3 center, vf3 hitbox)
{
	#if 0
	vf2 bottom_left = (project(center - vf3 { hitbox.x / 2.0f, hitbox.y / 2.0f, 0.0f })) * PIXELS_PER_METER;
	vf2 top_right   = (project(center + vf3 { hitbox.x / 2.0f, hitbox.y / 2.0f, 0.0f })) * PIXELS_PER_METER;

	SDL_Point points[] =
		{
			{ static_cast<i32>(bottom_left.x), static_cast<i32>(WINDOW_DIMENSIONS.y - 1.0f - bottom_left.y) },
			{ static_cast<i32>(top_right.x  ), static_cast<i32>(WINDOW_DIMENSIONS.y - 1.0f - bottom_left.y) },
			{ static_cast<i32>(top_right.x  ), static_cast<i32>(WINDOW_DIMENSIONS.y - 1.0f - top_right.y  ) },
			{ static_cast<i32>(bottom_left.x), static_cast<i32>(WINDOW_DIMENSIONS.y - 1.0f - top_right.y  ) },
			{ static_cast<i32>(bottom_left.x), static_cast<i32>(WINDOW_DIMENSIONS.y - 1.0f - bottom_left.y) }
		};

	SDL_RenderDrawLines(renderer, points, ARRAY_CAPACITY(points));
	#else
	constexpr f32 THICKNESS = 5.0f;
	vf2 bottom_left = (project(center - vf3 { hitbox.x / 2.0f, hitbox.y / 2.0f, 0.0f })) * PIXELS_PER_METER;
	vf2 dimensions  = (project(center + vf3 { hitbox.x / 2.0f, hitbox.y / 2.0f, 0.0f })) * PIXELS_PER_METER - bottom_left;

	draw_rect(renderer, bottom_left, { THICKNESS, dimensions.y });
	draw_rect(renderer, bottom_left + vf2 { dimensions.x - THICKNESS, 0.0f }, { THICKNESS, dimensions.y });
	draw_rect(renderer, bottom_left, { dimensions.x, THICKNESS });
	draw_rect(renderer, bottom_left + vf2 { 0.0f, dimensions.y - THICKNESS }, { dimensions.x, THICKNESS });
	#endif
}
