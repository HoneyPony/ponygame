#pragma once

#include "pony_string.h"
#include "pony_list.h"
#include "pony_benchtime.h"

// File scanning
typedef struct {
	list_of(str) c_paths;
	list_of(str) h_paths;
	list_of(str) png_paths;
	list_of(str) tex_paths;
	list_of(str) pony_paths;
} PathList;

extern PathList scan_for_files();

typedef struct {
	list_of(str) include_paths;
} Config;

void save_path_list(PathList *list);
void make_ninja_file(PathList *list, Config *config);
PathList load_path_list();

Config load_config();

// Directory tree
void ensure_directory_tree_exists();

// Image processing
void pack_images(const char **tex_paths, size_t path_count);

typedef struct {
	unsigned char *data;

	// Used when generating the ninja files...
	str image_source;
	str aseprite_source;

	size_t x;
	size_t y;
	size_t width;
	size_t height;

	size_t data_stride; // i.e. width of the loaded image

	str anim_name;
	int anim_frame;
	int anim_length;
} TexFileInfo;

typedef struct {
	str name;
	int frame_count;
} AnimInfo;

typedef struct {
	str image_source;
	str aseprite_source;
} TexBuildInfo;

void load_tex_file(const char *path, list_of(TexFileInfo) *output, list_of(AnimInfo) *anim_output);
TexBuildInfo load_tex_build_info(const char *path);