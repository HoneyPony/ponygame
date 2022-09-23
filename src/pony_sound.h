#pragma once

typedef struct {
	struct Mix_Chunk *chunk;
} SoundHandle;

typedef struct {
	struct _Mix_Music *music;
} MusicHandle;

void music_play(MusicHandle mus);
void sound_play(SoundHandle sound);
void set_volume(float volume);

extern SoundHandle fs_load_sound(const char *path);
extern MusicHandle fs_load_music(const char *path);