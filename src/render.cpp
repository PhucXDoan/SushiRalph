internal vf4 monochrome(f32 k, f32 a = 1.0f)
{
	return { k, k, k, a };
}

internal void set_color(SDL_Renderer* renderer, vf4 rgba)
{
	rgba *= 255.0f;
	SDL_SetRenderDrawColor(renderer, static_cast<u8>(rgba.x), static_cast<u8>(rgba.r), static_cast<u8>(rgba.b), static_cast<u8>(rgba.a));
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

internal inline void draw_line(SDL_Renderer* renderer, vf2 start, vf2 end)
{
	SDL_RenderDrawLine(renderer, static_cast<i32>(start.x), static_cast<i32>(WINDOW_DIMENSIONS.y - 1.0f - start.y), static_cast<i32>(end.x), static_cast<i32>(WINDOW_DIMENSIONS.y - 1.0f - end.y));
}

internal void draw_rect(SDL_Renderer* renderer, vf2 bottom_left, vf2 dimensions)
{
	SDL_Rect rect = { static_cast<i32>(bottom_left.x), static_cast<i32>(WINDOW_DIMENSIONS.y - 1.0f - bottom_left.y - dimensions.y), static_cast<i32>(dimensions.x), static_cast<i32>(dimensions.y) };
	SDL_RenderFillRect(renderer, &rect);
}
