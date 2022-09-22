#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pony.h"

void scan() {
	BTime time = bt_start();

	PathList sr = scan_for_files();
	Config config = load_config();
	save_path_list(&sr);
	make_ninja_file(&sr, &config);

	printf("scan completed in %fms\n", bt_passed_ms(time));
}

void build() {
	system("ninja");
}

void texpack(int argc, char **argv) {
	//pack_images(argv + 2, argc - 2);
}

extern void test_tex(const char *path);
extern void test_dir_tree();

int main(int argc, char **argv) {
	if(argc < 2) {
		puts("please put 'scan' or 'build', or 'go'");
		return -1;
	}

	if(!strcmp(argv[1], "scan")) {
		scan();
	}

	if(!strcmp(argv[1], "build")) {
		build();
	}

    if(!strcmp(argv[1], "go")) {
        build();
        system("game.exe");
    }

	if(!strcmp(argv[1], "print-tex")) {
		test_tex(argv[2]);
	}

    if(!strcmp(argv[1], "print-dir")) {
        test_dir_tree();
    }

	if(!strcmp(argv[1], "texpack")) {
		texpack(argc, argv);
	}

    if(!strcmp(argv[1], "build-res")) {
        puts("Rebuilding resources....");
        rebuild_resources();
        puts("Done!");
    }

	return 0;
}