#include <windows.h>
#include <sys/stat.h>
#include <stdio.h>
#include "platform.h"

internal time_t get_file_modification_time(strlit file_path)
{
	struct stat file_status;
	return
		stat(file_path, &file_status)
			? time_t {}
			: file_status.st_mtime;
}

internal void reload_program_dll(HotloadingData* hotloading_data)
{
	if (hotloading_data->dll)
	{
		SDL_UnloadObject(hotloading_data->dll);
	}

	SDL_RWops* src      = SDL_RWFromFile(PROGRAM_DLL_FILE_PATH     , "r");
	SDL_RWops* des      = SDL_RWFromFile(PROGRAM_DLL_TEMP_FILE_PATH, "w");
	i64        src_size = SDL_RWsize(src);
	byte*      buffer   = reinterpret_cast<byte*>(SDL_calloc(1, src_size));

	SDL_RWread (src, buffer, src_size, 1);
	SDL_RWwrite(des, buffer, src_size, 1);
	SDL_RWclose(src);
	SDL_RWclose(des);
	SDL_free(buffer);

	hotloading_data->dll                   = reinterpret_cast<byte*>(SDL_LoadObject(PROGRAM_DLL_TEMP_FILE_PATH));
	hotloading_data->dll_modification_time = get_file_modification_time(PROGRAM_DLL_FILE_PATH);
	hotloading_data->initialize            = reinterpret_cast<PrototypeUpdate*>(SDL_LoadFunction(hotloading_data->dll, "initialize"));
	hotloading_data->boot_down             = reinterpret_cast<PrototypeUpdate*>(SDL_LoadFunction(hotloading_data->dll, "boot_down"));
	hotloading_data->boot_up               = reinterpret_cast<PrototypeUpdate*>(SDL_LoadFunction(hotloading_data->dll, "boot_up"));
	hotloading_data->update                = reinterpret_cast<PrototypeUpdate*>(SDL_LoadFunction(hotloading_data->dll, "update"));
}

int main(int, char**)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		DEBUG_printf("SDL_Error: '%s'\n", SDL_GetError());
		ASSERT(!"SDL could not initialize video.");
		exit(-1);
	}

	if (TTF_Init() == -1)
	{
		DEBUG_printf("TTF_Error: '%s'\n", TTF_GetError());
		ASSERT(!"SDL_ttf could not initialize.");
		exit(-1);
	}

	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 8, 512) == -1)
	{
		DEBUG_printf("MIX_Error: '%s'\n", Mix_GetError());
		ASSERT(!"SDL_mixer could not initialize.");
		exit(-1);
	}

	SDL_Window* window = SDL_CreateWindow("Sushi Ralph", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, static_cast<i32>(WINDOW_DIMENSIONS.x), static_cast<i32>(WINDOW_DIMENSIONS.y), 0);
	if (!window)
	{
		DEBUG_printf("SDL_Error: '%s'\n", SDL_GetError());
		ASSERT(!"SDL could not create window.");
		exit(-1);
	}

	SDL_Renderer* window_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!window_renderer)
	{
		DEBUG_printf("SDL_Error: '%s'\n", SDL_GetError());
		ASSERT(!"SDL could not create a renderer for the window.");
		exit(-1);
	}

	if (SDL_SetRenderDrawBlendMode(window_renderer, SDL_BLENDMODE_BLEND))
	{
		DEBUG_printf("SDL_Error: '%s'\n", SDL_GetError());
		ASSERT(!"SDL could not set the blend mode for the renderer.");
		exit(-1);
	}

	Program program;
	program.is_running      = true;
	program.delta_seconds   = 0.0f;
	program.renderer        = window_renderer;
	program.memory          = reinterpret_cast<byte*>(VirtualAlloc(reinterpret_cast<LPVOID>(TEBIBYTES_OF(4)), MEMORY_CAPACITY, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
	program.memory_capacity = MEMORY_CAPACITY;

	HotloadingData hotloading_data = {};
	reload_program_dll(&hotloading_data);
	hotloading_data.initialize(&program);
	hotloading_data.boot_up(&program);

	u64 performance_count = SDL_GetPerformanceCounter();
	while (program.is_running)
	{
		u64 new_performance_count = SDL_GetPerformanceCounter();
		program.delta_seconds = static_cast<f32>(new_performance_count - performance_count) / SDL_GetPerformanceFrequency();
		performance_count = new_performance_count;

		if (get_file_modification_time(PROGRAM_DLL_FILE_PATH) != hotloading_data.dll_modification_time)
		{
			WIN32_FILE_ATTRIBUTE_DATA attributes_;
			while (GetFileAttributesEx(LOCK_FILE_PATH, GetFileExInfoStandard, &attributes_));

			hotloading_data.boot_down(&program);
			reload_program_dll(&hotloading_data);
			hotloading_data.boot_up(&program);
		}
		else
		{
			hotloading_data.update(&program);
		}

		SDL_Delay(1);
	}

	hotloading_data.boot_down(&program);
	SDL_DestroyRenderer(window_renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();

	return 0;
}
