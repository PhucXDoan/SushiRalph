#include <windows.h>
#include <sys/stat.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_FontCache.h>
#include <SDL_Mixer.h>
#include "platform.h"

#if DEBUG
namespace Debug
{
	global constexpr strlit DLL_FILE_PATH      = EXE_DIR "SushiRalph.dll";
	global constexpr strlit DLL_TEMP_FILE_PATH = EXE_DIR "SushiRalph.dll.temp"; // @TODO@ Do files get written in the exe file directory or the data directory?

	global byte*                dll;
	global time_t               dll_modification_time;
	global PrototypeInitialize* initialize;
	global PrototypeBootDown*   boot_down;
	global PrototypeBootUp*     boot_up;
	global PrototypeUpdate*     update;

	internal time_t fetch_dll_modification_time(void)
	{
		struct stat file_status;
		return
			stat(DLL_FILE_PATH, &file_status)
				? time_t {}
				: file_status.st_mtime;
	}

	internal void reload_dll(void)
	{
		if (dll)
		{
			SDL_UnloadObject(dll);
		}

		SDL_RWops* src      = SDL_RWFromFile(DLL_FILE_PATH     , "r");
		SDL_RWops* des      = SDL_RWFromFile(DLL_TEMP_FILE_PATH, "w");
		i64        src_size = SDL_RWsize(src);
		byte*      buffer   = reinterpret_cast<byte*>(SDL_calloc(1, src_size));

		SDL_RWread (src, buffer, src_size, 1);
		SDL_RWwrite(des, buffer, src_size, 1);
		SDL_RWclose(src);
		SDL_RWclose(des);
		SDL_free(buffer);

		dll                   = reinterpret_cast<byte*>(SDL_LoadObject(DLL_TEMP_FILE_PATH));
		dll_modification_time = fetch_dll_modification_time();
		initialize            = reinterpret_cast<PrototypeInitialize*>(SDL_LoadFunction(dll, "initialize"));
		boot_down             = reinterpret_cast<PrototypeBootDown*>  (SDL_LoadFunction(dll, "boot_down"));
		boot_up               = reinterpret_cast<PrototypeBootUp*>    (SDL_LoadFunction(dll, "boot_up"));
		update                = reinterpret_cast<PrototypeUpdate*>    (SDL_LoadFunction(dll, "update"));
	}
};
#endif

int main(int, char**)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		DEBUG_printf("SDL_Error: '%s'\n", SDL_GetError());
		ASSERT(false);
		exit(-1);
	}
	DEFER { SDL_Quit(); };

	if (TTF_Init() == -1)
	{
		DEBUG_printf("TTF_Error: '%s'\n", TTF_GetError());
		ASSERT(false);
		exit(-1);
	}
	DEFER { TTF_Quit(); };

	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 8, 512) == -1)
	{
		DEBUG_printf("MIX_Error: '%s'\n", Mix_GetError());
		ASSERT(false);
		exit(-1);
	}
	DEFER { Mix_CloseAudio(); };

	SDL_Window* window = SDL_CreateWindow("Sushi Ralph", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, static_cast<i32>(WINDOW_DIMENSIONS.x), static_cast<i32>(WINDOW_DIMENSIONS.y), 0);
	DEFER { SDL_DestroyWindow(window); };
	if (!window)
	{
		DEBUG_printf("SDL_Error: '%s'\n", SDL_GetError());
		ASSERT(false);
		exit(-1);
	}

	SDL_Renderer* window_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	DEFER { SDL_DestroyRenderer(window_renderer); };
	if (!window_renderer)
	{
		DEBUG_printf("SDL_Error: '%s'\n", SDL_GetError());
		ASSERT(false);
		exit(-1);
	}

	if (SDL_SetRenderDrawBlendMode(window_renderer, SDL_BLENDMODE_BLEND))
	{
		DEBUG_printf("SDL_Error: '%s'\n", SDL_GetError());
		ASSERT(false);
		exit(-1);
	}

	Platform platform;
	platform.is_running      = true;
	platform.delta_seconds   = 0.0f;
	platform.renderer        = window_renderer;
	platform.memory_capacity = MEBIBYTES_OF(1);
	platform.memory          = reinterpret_cast<byte*>(malloc(platform.memory_capacity));
	DEFER { free(platform.memory); };

	#if DEBUG
	Debug::reload_dll();
	DEFER { SDL_UnloadObject(Debug::dll); };
	Debug::initialize(&platform);
	Debug::boot_up(&platform);
	DEFER { Debug::boot_down(&platform); };
	#else
	initialize(&platform);
	boot_up(&platform);
	DEFER { boot_down(&platform); };
	#endif

	u64 performance_count = SDL_GetPerformanceCounter();
	while (platform.is_running)
	{
		u64 new_performance_count = SDL_GetPerformanceCounter();
		platform.delta_seconds = static_cast<f32>(new_performance_count - performance_count) / SDL_GetPerformanceFrequency();
		performance_count = new_performance_count;

		#if DEBUG
		if (Debug::fetch_dll_modification_time() != Debug::dll_modification_time)
		{
			WIN32_FILE_ATTRIBUTE_DATA attributes_;
			while (GetFileAttributesEx(EXE_DIR "LOCK.tmp", GetFileExInfoStandard, &attributes_));

			Debug::boot_down(&platform);
			Debug::reload_dll();
			Debug::boot_up(&platform);
		}
		else
		{
			Debug::update(&platform);
		}
		#else
		update(&platform);
		#endif

		SDL_Delay(1);
	}

	return 0;
}
