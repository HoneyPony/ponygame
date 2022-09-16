#include <dirent.h>
#include <unistd.h>

#include <stdio.h>

#include "pony.h"

bool already_exists(const char *path) {
	return access(path, F_OK) == 0;
}

void generate_empty_tex(const char *path) {
	FILE *out = fopen(path, "w");
	fputs("# This .tex file contains animation data for the associated texture.\n", out);
}

void scan_path(const char *path, PathList *result);

void scan_dir(const char *root_path, DIR* dir, PathList *result) {
	struct dirent *dir_entry;

	while ((dir_entry = readdir(dir)) != NULL) {
		if(!strcmp(dir_entry->d_name, ".")) continue;
		if(!strcmp(dir_entry->d_name, "..")) continue;

		str path = str_from(root_path);
		str_push(path, '/');
		str_append_cstr(path, dir_entry->d_name);
		scan_path(path, result);
		str_free(path);
	}
	closedir(dir);
}



void scan_path(const char *path, PathList *result) {
	DIR *dir;
	
	dir = opendir(path);
	// If the path is a directory, scan for files inside the directory.
	if (dir) {
		scan_dir(path, dir, result);
		return;
	}

	// The path is not a directory. Check if it has one of the extensions we're
	// looking for.
	#define IF_SUFFIX(suffix) if(cstr_has_suffix(path, "." #suffix))
	#define PUSH_SUFFIX(suffix) ls_push(result-> suffix ## _paths, str_from(path))
	#define CHECK_SUFFIX(suffix) IF_SUFFIX(suffix) { PUSH_SUFFIX(suffix); }
	
	IF_SUFFIX(c) {
		if(!cstr_has_suffix(path, ".pony.c")) {
			PUSH_SUFFIX(c);
		}
	}
	
	CHECK_SUFFIX(png)

	// Already existing tex files get pushed...
	CHECK_SUFFIX(tex)
	
	CHECK_SUFFIX(pony)
}

void generate_new_files(PathList *result) {
	foreach(png_file, result->png_paths, {
		str tex_name = str_from(png_file);

		// Already know it ends in .png, so this is safe
		str_rewind(tex_name, 3);
		str_append_cstr(tex_name, "tex");

		// Generate the tex file if it doesn't already exist.
		if(!already_exists(tex_name)) {
			generate_empty_tex(tex_name);
			ls_push(result->tex_paths, tex_name);
		}
		else {
			str_free(tex_name);
		}
	})
}

void fix_paths(list_of(str) list) {
	foreach(path, list, {
		if(cstr_has_prefix(path, "./"))
			str_trim_front(path, 2);
	})
}

PathList scan_for_files() {
	PathList result;
	ls_init(result.c_paths);
	ls_init(result.h_paths);
	ls_init(result.png_paths);
	ls_init(result.tex_paths);
	ls_init(result.pony_paths);

	scan_path(".", &result);

	fix_paths(result.c_paths);
	fix_paths(result.h_paths);
	fix_paths(result.png_paths);
	fix_paths(result.tex_paths);
	fix_paths(result.pony_paths);

	generate_new_files(&result);

	return result;
}