#include <stdio.h>
#include <string.h>

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

int main(int argc, char **argv) {
	if(argc < 2) {
		puts("please put 'scan' or 'build'");
		return -1;
	}

	if(!strcmp(argv[1], "scan")) {
		scan();
	}

	if(!strcmp(argv[1], "build")) {
		build();
	}

	return 0;
}