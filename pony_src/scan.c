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
	fclose(out);
}

void generate_png_tex(const char *tex_path, const char* png_file) {
	FILE *out = fopen(tex_path, "w");
	fputs("# This .tex file contains animation data for the associated texture.\n", out);
	fprintf(out, "@load %s\n", png_file);

	fclose(out);
}

void generate_aseprite_tex(const char *tex_path, const char* aseprite_file) {
	str png_file = str_from(tex_path);
	str_append_cstr(png_file, ".png"); // File format: ".tex.png"

	FILE *out = fopen(tex_path, "w");
	fputs("# This .tex file contains animation data for the associated texture.\n", out);
	fprintf(out, "@load %s\n", png_file);
	fprintf(out, "@from-aseprite %s\n", aseprite_file);

	fclose(out);

	str_free(png_file);
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

	// Do not scan '.' paths.
	if(cstr_has_prefix(path, "./.")) return;
    // Do not scan the ponygame path.
    //if(cstr_has_prefix(path, "./.ponygame") || cstr_has_prefix(path, ".ponygame")) return;
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
	
	CHECK_SUFFIX(c)

	// Previously it seemed like it might be better not to add the .pony.c files
	// because they are being added elsewhere, but honestly, this seems fine to me...
	//
	// It even lets you delete a .pony file but keep the C file if you want. Seems
	// more straightforward than special-casing it.
	//
	// The only weird thing will be adding those C files to the ninja file... but
	// even that can be handled just by adding them to the list when they are first
	// generated.
	/*IF_SUFFIX(c) {
		if(!cstr_has_suffix(path, ".pony.c")) {
			PUSH_SUFFIX(c);
		}
	}*/
	
	CHECK_SUFFIX(png)

	// Already existing tex files get pushed...
	CHECK_SUFFIX(tex)
	
	CHECK_SUFFIX(pony)

	CHECK_SUFFIX(aseprite)

	CHECK_SUFFIX(snd)
}

bool already_has_png(PathList *result, const char *name) {
	foreach_r(tb, result->tex_infos, {
		if(!tb->image_source) continue;

		// If this image is already in a source, we already have it
		if(!strcmp(name, tb->image_source)) {
			return true;
		}
	})
	// Don't already have the PNG
	return false;
}

bool already_has_aseprite(PathList *result, const char *name) {
	foreach_r(tb, result->tex_infos, {
		if(!tb->aseprite_source) continue;

		// If this image is already in a source, we already have it
		if(!strcmp(name, tb->aseprite_source)) {
			return true;
		}
	})
	// Don't already have the PNG
	return false;
}

void generate_tex_from_aseprite(PathList *result, const char *source) {
	str tex_name = str_from(source);

	str_rewind(tex_name, 8); // remove 'aseprite'
	str_append_cstr(tex_name, "tex");

	if(!already_exists(tex_name)) {
		generate_aseprite_tex(tex_name, source);
		ls_push(result->tex_paths, tex_name);

		// Load the new build info
		TexBuildInfo tb = load_tex_build_info(tex_name);
		ls_push_var(result->tex_infos, tb);
	}
	else {
		str_free(tex_name);
	}
}

void generate_tex_from_png(PathList *result, const char *png_file) {
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
}

void generate_new_files(PathList *result) {
	foreach(aseprite_file, result->aseprite_paths, {
		// We don't already have a .tex loading this aseprite
		if(!already_has_aseprite(result, aseprite_file)) {
			generate_tex_from_aseprite(result, aseprite_file);
		}
	})

	foreach(png_file, result->png_paths, {
		if(!already_has_png(result, png_file)) {
			generate_tex_from_png(result, png_file);
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
	ls_init(result.aseprite_paths);
	ls_init(result.snd_paths);

	ls_init(result.tex_infos);
	ls_init(result.snd_infos);

	scan_path(".", &result);

	fix_paths(result.c_paths);
	fix_paths(result.h_paths);
	fix_paths(result.png_paths);
	fix_paths(result.tex_paths);
	fix_paths(result.pony_paths);
	fix_paths(result.aseprite_paths);
	fix_paths(result.snd_paths);

	foreach(tex_path, result.tex_paths, {
		TexBuildInfo tb = load_tex_build_info(tex_path);
		ls_push_var(result.tex_infos, tb);
	})

	foreach(snd_path, result.snd_paths, {
		SndBuildInfo sb = load_snd_build_info(snd_path);
		ls_push_var(result.snd_infos, sb);
	})

	generate_new_files(&result);

	return result;
}