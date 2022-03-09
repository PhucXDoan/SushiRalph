#pragma once
#include "platform.h"

global constexpr f32 SECONDS_PER_UPDATE = 1.0f / 30.0f;

global constexpr f32 BELT_SPACING     = WINDOW_DIMENSIONS.y / 6.0f;
global constexpr f32 BELT_HEIGHT      = WINDOW_DIMENSIONS.y / 3.0f;
global constexpr f32 BELT_LIGHTNESS[] = { 0.6f, 0.575f, 0.625f };

global constexpr f32    TITLE_MENU_OPTION_SPACING = 250.0f;
global constexpr strlit TITLE_MENU_OPTIONS[] =
	{
		"Play",
		"Options",
		"Credits",
		"Exit"
	};

struct Input
{
	bool8 left;
	bool8 right;
	bool8 down;
	bool8 up;
	bool8 accept;
};

struct State
{
	f32   seconds_accumulated;
	Input input;
	Input prev_input;

	f32   belt_offsets[3];
	i32   title_menu_option_index;

	FC_Font* font;
};
