#pragma once
#include "platform.h"

global constexpr f32 SECONDS_PER_UPDATE = 1.0f / 30.0f;

struct Input
{
	bool8 down;
	bool8 up;
	bool8 accept;
};

struct State
{
	f32   seconds_accumulated;
	Input input;
	Input prev_input;

	FC_Font* font;
};
