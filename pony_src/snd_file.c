#include <stdio.h>

#include "pony.h"

SndBuildInfo load_snd_build_info(const char *path) {
	FILE *in = fopen(path, "r");

	SndBuildInfo info = { NULL, NULL, NULL, SND_BUILD_OGG, false };

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

	
	if(cstr_has_prefix(path, "./")) {
		// TODO: Figure out if this is right
		info.snd_path = str_from(path + 2);
	}
	else {
		info.snd_path = str_from(path);
	}

	if(info.sound_source) {
		info.build_output = str_from(".ponygame/build.resources/");
		str_append_str(info.build_output, info.snd_path); // Build is based on the '.snd' file
		str_append_cstr(info.build_output, ".ogg"); // TODO: Implement other sound types
	}

	

	fclose(in);

	return info;
}
