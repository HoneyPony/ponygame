//#ifdef __EMSCRIPTEN__
//#include <SDL/SDL_mixer.h>
//#else
#include <SDL2/SDL_mixer.h>
//#endif

#include "pony_sound.h"
#include "pony_log.h"
#include "pony_string.h"

SoundHandle fs_load_sound(const char *path) {
	if(cstr_has_prefix(path, "res://")) {
		/* TODO: Implement loading from memory in final builds */
		path += 6;
	}

	return (SoundHandle){Mix_LoadWAV(path)};
}

MusicHandle fs_load_music(const char *path) {
	const char *src_path = path;
	if(cstr_has_prefix(path, "res://")) {
		/* TODO: Implement loading from memory in final builds */
		path += 6;
	}

	MusicHandle result = {Mix_LoadMUS(path)};
	if(!result.music) {
		logf_error("failed to load music file: %s", src_path);
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