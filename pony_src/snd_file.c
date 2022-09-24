#include <stdio.h>

#include "pony.h"

SndBuildInfo load_snd_build_info(const char *path) {
	FILE *in = fopen(path, "r");

	SndBuildInfo info = { NULL, NULL, false };

	char line[1024];
	char arg[1024];
	while(!feof(in)) {
		line[0] = 0;
		arg[0] = 0;
		fgets(line, 1024, in);
		if(sscanf(line, "@load %s", arg) == 1) {
			info.sound_source = str_from(arg);
		}
		if(cstr_has_prefix(line, "@music")) {
			info.is_music = true;
		}
	}

	info.snd_path = str_from(path);

	fclose(in);

	return info;
}
