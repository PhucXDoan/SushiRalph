void set_color(SDL_Renderer* renderer, vf4 rgba)
{
	rgba *= 255.0f;
	SDL_SetRenderDrawColor(renderer, static_cast<u8>(rgba.x), static_cast<u8>(rgba.r), static_cast<u8>(rgba.b), static_cast<u8>(rgba.a));
}

template <typename... ARGUMENTS>
void draw_text(SDL_Renderer* renderer, FC_Font* font, vf2 coordinates, FC_AlignEnum alignment, f32 scalar, vf4 rgba, strlit fstr, ARGUMENTS... parameters)
{
	rgba *= 256.0f;
	FC_DrawEffect
	(
		font,
		renderer,
		coordinates.x,
		WINDOW_DIMENSIONS.y - 1.0f - coordinates.y - FC_GetBaseline(font),
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
