#include <stdio.h>
#include <stdlib.h>

#include "pony.h"

void process_krita_from_texbuild(TexBuildInfo *tb) {
	char krita_cmd[2048] = {0};
	// TODO: Support configurable Krita path

	// Args:
	// -> .kra file to process
	// -> folder to save .tmp pngs in
	// -> output name for spritesheet .png
	// -> output name for tex
	snprintf(krita_cmd, 2048, "\"C:\\Program Files\\Krita (x64)\\bin\\kritarunner.com\" -s ponygame_krita_exporter %s %s %s %s",
		tb->krita_source, ".ponytmp", tb->image_source, tb->tex_path);

	// The krita script will itself generate the ".tex" file as well as
	// the spritesheet by running image magick.
	//
	// Very hacked together. But it should work!
    system(krita_cmd);
}

void process_krita_from_tex(const char *path) {
	TexBuildInfo tb = load_tex_build_info(path);
	process_krita_from_texbuild(&tb);
}