#pragma once
#include "platform.h"
#include "unified.h"

struct ObstacleAsset
{
	strlit file_path;
	f32    scalar;
	vf2    center;
	vf3    hitbox;
};

global constexpr strlit        SAVE_DATA_FILE_PATH = DATA_DIR "SushiRalph.save";
global constexpr ObstacleAsset OBSTACLE_ASSETS[]   =
	{
		{ DATA_DIR "sushi_0.bmp", 0.15f, { 0.5f, 0.6f }, { 0.69f, 0.5f, 0.2f } },
		{ DATA_DIR "sushi_1.bmp", 0.15f, { 0.5f, 0.6f }, { 0.50f, 0.5f, 0.2f } },
		{ DATA_DIR "sushi_2.bmp", 0.10f, { 0.5f, 0.7f }, { 0.49f, 0.5f, 0.2f } },
		{ DATA_DIR "sushi_3.bmp", 0.15f, { 0.5f, 0.5f }, { 1.25f, 1.0f, 0.2f } }
	};

global constexpr f32 SECONDS_PER_UPDATE  = 1.0f / 60.0f;
global constexpr f32 PIXELS_PER_METER    = 128.0f;
global constexpr f32 GRAVITY             = -9.81f;
global constexpr f32 CALORIES_PER_METER  = 2.5f;
global constexpr f32 CALORIES_PER_SWITCH = 1.0f;
global constexpr f32 CALORIES_PER_JUMP   = 4.0f;

global constexpr i32 BELT_COUNT                = 3;
global constexpr f32 BELT_SPACING              = 0.5f;
global constexpr f32 BELT_HEIGHT               = 0.75f;
global constexpr f32 BELT_LIGHTNESS[]          = { 0.6f, 0.575f, 0.625f };
global constexpr f32 BELT_MAX_SPEED            = 1.0f;
global constexpr f32 BELT_MIN_SPEED            = 6.0f;

global constexpr vf3 RALPH_HITBOX_DIMENSIONS   = { 0.6f, 0.6f, 0.075f };
global constexpr f32 RALPH_X                   = 4.0f;

global constexpr f32    TITLE_MENU_OPTION_SPACING = 2.0f;
global constexpr strlit TITLE_MENU_OPTIONS[]      = { "Play", "Settings", "Credits", "Exit" };
global constexpr f32    TITLE_MENU_OPTIONS_WIDTH  = WINDOW_DIMENSIONS.x / PIXELS_PER_METER * 0.75f;

global constexpr f32    SETTINGS_OPTIONS_OFFSET      = TITLE_MENU_OPTIONS_WIDTH + 5.0f;
global constexpr f32    SETTINGS_OPTION_SPACING      = 3.0f;
global constexpr strlit SETTINGS_OPTIONS[]           = { "Back", "Master volume", "Music volume", "SFX volume" };
global constexpr f32    SETTINGS_VOLUME_SLIDE_OFFSET = 7.5f;
global constexpr f32    SETTINGS_VOLUME_SLIDE_SPEED  = 0.5f;
global constexpr f32    SETTINGS_VOLUME_SLIDE_WIDTH  = 5.0f;

global constexpr f32    CREDITS_OFFSET = WINDOW_DIMENSIONS.x / PIXELS_PER_METER / 2.0f + 2.0f;
global constexpr f32    CREDIT_SPACING = 5.0f;
global constexpr strlit CREDITS[]      = { "Programming : Phuc Doan", "Art : Mila Matthews" };

global constexpr f32    GAME_OVER_STATS_OFFSET = 25.0f;

enum_struct (AudioChannel, u8)
{
	background_music = 0,
	background_music_muffled
};

// @TODO@ Use an actual RNG lol.
// @STICKY@ Is in interval [0, 65536).
global constexpr u16 RAND_TABLE[] =
	{
		0x9b9d, 0x4e65, 0x8ec9, 0x30e9, 0x5477, 0xe845, 0xab16, 0x910d,
		0xfdf3, 0x73dc, 0x424b, 0xc8f9, 0x27a1, 0xe07a, 0x6803, 0xb8c1,
		0x9525, 0x1c29, 0xcf1f, 0x3e18, 0xc463, 0x165f, 0x7018, 0xeae4,
		0x7e41, 0x945e, 0xbbbf, 0x5ac7, 0x15a4, 0xa666, 0xa8f1, 0xbc54,
		0xe604, 0x3393, 0x5cad, 0x061f, 0x98b7, 0x93fd, 0xc762, 0xa37b,
		0x1880, 0x26cd, 0xb131, 0x3f59, 0xae82, 0x264f, 0x9355, 0xc0e8,
		0x5e46, 0x76bd, 0xd8f2, 0x2e69, 0xb158, 0x0317, 0x127f, 0x936a,
		0x0ccd, 0x8497, 0xd08d, 0x92ad, 0xa3d3, 0xcf59, 0x1dc6, 0xbb4b,
		0x7ecb, 0xb664, 0x93c7, 0xd3f1, 0xd23c, 0x9b2f, 0x5fae, 0x8c17,
		0xdfee, 0xd19c, 0x3bff, 0x6ac7, 0x078c, 0x8282, 0x9fe3, 0xa7fd,
		0xa653, 0xd182, 0x31ad, 0x0938, 0x5ac8, 0xcd79, 0x8622, 0x4e64,
		0xeb1f, 0x3da6, 0x133d, 0x6f45, 0xe061, 0x6449, 0xe19e, 0xf63b,
		0x5440, 0xe667, 0x9be6, 0x27f1, 0xfbee, 0x407b, 0x3c76, 0x54b6,
		0x8061, 0x6e0b, 0x36f5, 0xdd0d, 0xbaa5, 0xe8dc, 0x2a46, 0x4f8f,
		0x63ab, 0xda6c, 0x5950, 0x1df6, 0xd32a, 0xf42b, 0x68dd, 0xe9d2,
		0xb825, 0x31b8, 0x2a7c, 0x49f0, 0xe859, 0x4041, 0x2fce, 0x72f9,
		0x8224, 0x9615, 0x888f, 0x2050, 0x9172, 0x5744, 0xe53a, 0xfaef,
		0xf191, 0xd627, 0x2dae, 0x69da, 0xe4b3, 0x4a30, 0x9436, 0x80cd,
		0xb863, 0x7ac0, 0xd72b, 0xb49c, 0x20fc, 0xa0c6, 0x29c2, 0xb683,
		0xc820, 0x36d8, 0xeb86, 0x3bc5, 0xd2f4, 0xa04e, 0x90b4, 0xe5f0,
		0x44df, 0x442b, 0xf29c, 0x72db, 0x0fdf, 0x366f, 0x36e7, 0x734a,
		0x00c9, 0x40c1, 0xbddf, 0x7fcd, 0xff1e, 0x31a9, 0xd998, 0x67c7,
		0x328f, 0xbf19, 0xaa42, 0x165a, 0x3975, 0x99fb, 0x0df0, 0xe0ac,
		0x6cf0, 0x6c4d, 0x7a87, 0x73b5, 0xc3fc, 0x8878, 0xe755, 0x01ab,
		0x2d9e, 0x19dc, 0x2f14, 0xcaca, 0x4510, 0x2983, 0xfa28, 0xd81a,
		0xd4ba, 0x757a, 0xfe60, 0x58fd, 0xb899, 0x24e8, 0x90ac, 0x1a92,
		0x4b26, 0xb050, 0xd8e1, 0x96d1, 0x6ad2, 0x330d, 0x9e1a, 0xcfc4,
		0xaabd, 0x0394, 0x1a64, 0x59da, 0x4f65, 0xac6f, 0x381d, 0x1c57,
		0xbc45, 0xab88, 0x0f90, 0x2dd3, 0x468b, 0x71ac, 0xe1d1, 0x4e64,
		0x1f1e, 0xbb90, 0xa3d7, 0xa04a, 0xd59b, 0x8778, 0x38ed, 0xd353,
		0x5e6f, 0xd907, 0xbbcf, 0xe53e, 0xe37a, 0x7c8d, 0x9d9d, 0xb302,
		0xda05, 0xd0df, 0x0cbf, 0x2efe, 0x38aa, 0xbd2d, 0x1a53, 0x0f2b
	};

struct Sprite
{
	SDL_Texture* texture;
	i32          width_pixels;
	i32          height_pixels;
	f32          scalar;
	vf2          center;
	i32          frame_index;
	i32          frame_count;
	f32          seconds_per_frame;
	f32          seconds_accumulated;
};

struct Obstacle
{
	i32 sprite_index;
	i32 belt_index;
	vf3 position;
};

struct SaveData
{
	f32 highest_calories_burned;
	f32 master_volume;
	f32 music_volume;
	f32 sfx_volume;
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
	settings,
	credits,
	playing,
	game_over
};

struct State
{
	f32       seconds_accumulated;
	Input     input;
	Input     prev_input;
	u32       seed;

	StateType type;
	f32       belt_offsets[3];
	f32       belt_velocities[3];
	f32       dampen_belt_velocities[3];
	f32       background_music_keytime;
	f32       dampen_background_music_keytime;

	struct
	{
		f32 resetting_keytime; // @TODO@ The resetting transition should be baked in the `game_over` state.
		f32 initial_belt_offsets[3];

		i32 option_index;
	} title_menu;

	struct
	{
		bool32 showing;
		f32    show_keytime;
		f32    initial_belt_offsets[3];
		bool32 changing_option;
		i32    option_index;
	} settings;

	struct
	{
		bool32 showing;
		f32    keytime;
		f32    initial_belt_offsets[3];
		i32    credit_index;
	} credits;

	struct
	{
		f32      belt_velocity_update_keytime;
		i32      ralph_belt_index;
		vf3      ralph_position;
		vf3      ralph_velocity;
		f32      target_ralph_velocity_x;
		Obstacle obstacles[8];
		f32      calories_burned;
		f32      peak_calories_burned;
		f32      time;
	} playing;

	struct
	{
		bool32 exiting;
		f32    keytime;
		f32    initial_belt_offsets[3];
		i32    stat_belt_index;
	} game_over;

	SaveData   save_data;
	FC_Font*   font;
	Sprite     ralph_running_sprite;
	Sprite     ralph_exploding_sprite;
	Sprite     shadow_sprite;
	Sprite     obstacle_sprites[ARRAY_CAPACITY(OBSTACLE_ASSETS)];
	Mix_Chunk* background_music;
	Mix_Chunk* background_music_muffled;
	Mix_Chunk* explosion_sfx;
	Mix_Chunk* chomp_sfx;
};
