#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pony.h"

void build() {
	system("ninja -v");
}

int main(int argc, char **argv) {
	if(argc < 2) {
		puts("please put 'scan' or 'build', or 'go'");
		return -1;
	}

	if(!strcmp(argv[1], "scan")) {
		rebuild_resources();
	}

	if(!strcmp(argv[1], "build")) {
		build();
	}

    if(!strcmp(argv[1], "go")) {
        build();
        system("game.exe");
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