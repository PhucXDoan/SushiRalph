@echo off

set DEBUG_COMMON_COMPILER_FLAGS=-DROOT_DIR="\"W:/\"" -DDEBUG=1 -nologo -std:c++17 -MTd -GR- -EHsc -EHa- -Od -Oi -Z7 -W4 -wd4702 -wd4100 -wd4201 -wd4127 -I W:\lib\SDL2\include\ -I W:\lib\SDL2_ttf\include\ -I W:\lib\SDL_FontCache\ -I W:\lib\SDL2_mixer\include\
set DEBUG_COMMON_LINKER_FLAGS=-DEBUG:FULL -opt:ref -incremental:no -subsystem:windows W:\lib\SDL2\lib\x64\SDL2.lib W:\lib\SDL2\lib\x64\SDL2main.lib W:\lib\SDL2_ttf\lib\x64\SDL2_ttf.lib W:\lib\SDL_FontCache\SDL_FontCache.lib W:\lib\SDL2_mixer\lib\x64\SDL2_mixer.lib shell32.lib

set RELEASE_COMMON_COMPILER_FLAGS=-DROOT_DIR="\"./\"" -nologo -std:c++17 -MTd -GR- -EHsc -EHa- -O2 -Z7 -W4 -wd4702 -wd4100 -wd4201 -wd4127 -I W:\lib\SDL2\include\ -I W:\lib\SDL2_ttf\include\ -I W:\lib\SDL_FontCache\ -I W:\lib\SDL2_mixer\include\
set RELEASE_COMMON_LINKER_FLAGS=-opt:ref -incremental:no -subsystem:windows W:\lib\SDL2\lib\x64\SDL2.lib W:\lib\SDL2\lib\x64\SDL2main.lib W:\lib\SDL2_ttf\lib\x64\SDL2_ttf.lib W:\lib\SDL_FontCache\SDL_FontCache.lib W:\lib\SDL2_mixer\lib\x64\SDL2_mixer.lib shell32.lib

IF NOT EXIST W:\lib\SDL_FontCache\SDL_FontCache.lib (
	pushd W:\lib\SDL_FontCache\
	cl  /c /EHsc -MTd -I W:\lib\SDL2\include\ -I W:\lib\SDL2_ttf\include\ W:\lib\SDL_FontCache\SDL_FontCache.c
	lib SDL_FontCache.obj
	popd
)

IF NOT EXIST W:\build\ mkdir W:\build\

pushd W:\build\
set DEBUG=1
if DEFINED DEBUG (
	del *.pdb > NUL 2> NUL
	echo "LOCK" > LOCK.tmp
	cl  %DEBUG_COMMON_COMPILER_FLAGS% -LD               W:\src\SushiRalph.cpp     /link %DEBUG_COMMON_LINKER_FLAGS% -PDB:SushiRalph%RANDOM%.pdb -EXPORT:initialize -EXPORT:boot_down -EXPORT:boot_up -EXPORT:update
	cl  %DEBUG_COMMON_COMPILER_FLAGS% -FeSushiRalph.exe W:\src\platform_WIN32.cpp /link %DEBUG_COMMON_LINKER_FLAGS%
	sleep 0.25
	del LOCK.tmp
) else (
	cl %RELEASE_COMMON_COMPILER_FLAGS% -LD               W:\src\SushiRalph.cpp     /link %RELEASE_COMMON_LINKER_FLAGS% -EXPORT:initialize -EXPORT:boot_down -EXPORT:boot_up -EXPORT:update
	cl %RELEASE_COMMON_COMPILER_FLAGS% -FeSushiRalph.exe W:\src\platform_WIN32.cpp /link %RELEASE_COMMON_LINKER_FLAGS%
)
popd
