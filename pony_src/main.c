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
		bool pack = false;
		if(argc >= 3 && !strcmp(argv[2], "--pack")) {
			pack = true;
		}
		rebuild_resources(pack);
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

	return 0;
}