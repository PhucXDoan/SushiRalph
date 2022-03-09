#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_FontCache.h>
#include "unified.h"

global constexpr strlit  PROGRAM_DLL_FILE_PATH      = "W:/build/SushiRalph.dll";
global constexpr strlit  PROGRAM_DLL_TEMP_FILE_PATH = "W:/build/SushiRalph.dll.temp";
global constexpr strlit  LOCK_FILE_PATH             = "W:/build/LOCK.tmp";
global constexpr memsize MEMORY_CAPACITY            = MEBIBYTES_OF(1);
global constexpr i32     WINDOW_WIDTH               = 1280;
global constexpr i32     WINDOW_HEIGHT              = 720;

struct Program
{
	bool32        is_running;
	f32           delta_seconds;
	SDL_Renderer* renderer;
	byte*         memory;
	memsize       memory_capacity;
};

#define PROTOTYPE_INITIALIZE(NAME) void NAME(Program* program)
typedef PROTOTYPE_INITIALIZE(PrototypeInitialize);

#define PROTOTYPE_BOOT_DOWN(NAME) void NAME(Program* program)
typedef PROTOTYPE_BOOT_DOWN(PrototypeBootDown);

#define PROTOTYPE_BOOT_UP(NAME) void NAME(Program* program)
typedef PROTOTYPE_BOOT_UP(PrototypeBootUp);

#define PROTOTYPE_UPDATE(NAME) void NAME(Program* program)
typedef PROTOTYPE_UPDATE(PrototypeUpdate);

struct HotloadingData
{
	byte*                dll;
	time_t               dll_modification_time;
	PrototypeInitialize* initialize;
	PrototypeBootDown*   boot_down;
	PrototypeBootUp*     boot_up;
	PrototypeUpdate*     update;
};
