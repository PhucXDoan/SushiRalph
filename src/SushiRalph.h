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

struct Sprite
{
	SDL_Texture* texture;
	i32          width_pixels;
	i32          height_pixels;
	f32          scalar;
	i32          frame_index;
	i32          frame_count;
	f32          seconds_per_frame;
	f32          seconds_accumulated;
};

struct Input
{
	bool8 left;
	bool8 right;
	bool8 down;
	bool8 up;
	bool8 accept;
};

enum_struct (StateType, u32)
{
	title_menu = 1,
	playing
};

struct State
{
	f32       seconds_accumulated;
	Input     input;
	Input     prev_input;

	StateType type;
	f32       belt_offsets[3];

	Sprite    sprite_ralph_running;

	struct
	{
		i32 option_index;
	} title_menu;

	struct
	{
	} playing;

	FC_Font* font;
};
