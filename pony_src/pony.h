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