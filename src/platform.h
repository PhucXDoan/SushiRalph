#pragma once
#include "unified.h"

global constexpr vf2 WINDOW_DIMENSIONS = vf2 ( 1280, 128 * 3 );

struct Platform
{
	bool32        is_running;
	f32           delta_seconds;
	SDL_Renderer* renderer;
	memsize       memory_capacity;
	byte*         memory;
};

#define PROTOTYPE_INITIALIZE(NAME) void NAME(Platform* platform)
typedef PROTOTYPE_INITIALIZE(PrototypeInitialize);

#define PROTOTYPE_BOOT_DOWN(NAME) void NAME(Platform* platform)
typedef PROTOTYPE_BOOT_DOWN(PrototypeBootDown);

#define PROTOTYPE_BOOT_UP(NAME) void NAME(Platform* platform)
typedef PROTOTYPE_BOOT_UP(PrototypeBootUp);

#define PROTOTYPE_UPDATE(NAME) void NAME(Platform* platform)
typedef PROTOTYPE_UPDATE(PrototypeUpdate);

#if DEBUG
#else
extern "C" PROTOTYPE_INITIALIZE(initialize);
extern "C" PROTOTYPE_BOOT_DOWN(boot_down);
extern "C" PROTOTYPE_BOOT_UP(boot_up);
extern "C" PROTOTYPE_UPDATE(update);
#endif
