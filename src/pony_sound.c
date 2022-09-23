//#ifdef __EMSCRIPTEN__
//#include <SDL/SDL_mixer.h>
//#else
#include <SDL2/SDL_mixer.h>
//#endif

#include "pony_sound.h"
#include "pony_log.h"
#include "pony_string.h"
#include "pony_fs.h"

static SDL_RWops *from_path(const char *path) {
	if(cstr_has_prefix(path, "res://")) {
		if(pony_resources_are_packed) {
			FSPackedMem mem = fs_find_packed_resource(path);
			return SDL_RWFromConstMem(mem.ptr, (int)mem.length);
		}
		else {
			path += 6;
			return SDL_RWFromFile(path, "r");
		}
	}

	return SDL_RWFromFile(path, "r");
}

SoundHandle fs_load_sound(const char *path) {
	SDL_RWops *ops = from_path(path);
	
	SoundHandle result = {Mix_LoadWAV_RW(ops, true)};
	
	return result;
}

MusicHandle fs_load_music(const char *path) {
	SDL_RWops *ops = from_path(path);

	MusicHandle result = {Mix_LoadMUS_RW(ops, true)};
	if(!result.music) {
		logf_error("failed to load music file: %s", path);
	}
	return result;
}

void sound_play(SoundHandle sound) {
	Mix_PlayChannel(-1, sound.chunk, 1);
}

void music_play(MusicHandle music) {
	Mix_PlayMusic(music.music, -1);
}

void set_volume(float volume) {
	int v = volume * MIX_MAX_VOLUME;
	Mix_Volume(-1, v);
	//Mix_MasterVolume(v);
}

void pony_init_sound() {
	int result = Mix_OpenAudio(
		MIX_DEFAULT_FREQUENCY,
		MIX_DEFAULT_FORMAT,
		MIX_DEFAULT_CHANNELS, 2048);

	if(result < 0) {
		logf_error("could not open audio device!");
	}
	else {
		logf_info("successfully opened audio device");
	}

	Mix_Volume(-1, MIX_MAX_VOLUME);
}