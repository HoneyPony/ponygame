#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pony.h"

void build(const char *file) {
	remove("game.exe");

	char cmd[256] = {0};
	snprintf(cmd, 256, "ninja -v -f %s", file);
	system(cmd);
}

int main(int argc, char **argv) {
	if(argc < 2) {
		puts("please put 'scan' or 'build', or 'go'");
		return -1;
	}

	if(!strcmp(argv[1], "scan")) {
		RebuildResourceArguments args = {0};
		for(int i = 2; i < argc; ++i) {
			if(!strcmp(argv[i], "--pack")) {
				args.pack = true;
			}
			// TODO: Implement this... will require storing the generated rectangles,
			// or something, so that the res_loader.c can still be generated
			//if(!strcmp(argv[i], "--no-sheet")) {
			//	args.no_sheet = true;
			//}
		}
		rebuild_resources(args);
	}

	if(!strcmp(argv[1], "build")) {
		build("build.debug.local.ninja");
	}

    if(!strcmp(argv[1], "go")) {
        build("build.debug.local.ninja");
        system("game.debug.local.exe");
    }

	if(!strcmp(argv[1], "generate:res_debug.c")) {
		generate_file_res_debug_c();
	}

	if(!strcmp(argv[1], "tex-from-aseprite")) {
		if(argc > 2) {
			printf("pony: tex-from-aseprite %s ", argv[2]);
			process_aseprite_from_tex(argv[2]);
			puts("done!");
		}
	}

	if(!strcmp(argv[1], "init")) {
		ensure_directory_tree_exists();
		generate_file_blank_ponyconfig();
		generate_file_blank_main_c();
	}

	return 0;
}