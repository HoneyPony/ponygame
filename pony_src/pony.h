#pragma once

#include "pony_string.h"
#include "pony_list.h"
#include "pony_benchtime.h"

typedef struct {
	str image_source;
	str aseprite_source;
	str tex_path;
} TexBuildInfo;

#define SND_BUILD_UNKNOWN 0
#define SND_BUILD_WAV 1
#define SND_BUILD_FLAC 2
#define SND_BUILD_OGG 3

typedef struct {
	str sound_source;
	str snd_path;

	str build_output;

	// What file type the sound included in the game distributable should be.
	int build_type;

	bool is_music;
} SndBuildInfo;

// File scanning
typedef struct {
	list_of(str) c_paths;
	list_of(str) h_paths;
	list_of(str) png_paths;
	list_of(str) aseprite_paths;
	list_of(str) tex_paths;
	list_of(str) pony_paths;
	list_of(str) snd_paths;

	list_of(TexBuildInfo) tex_infos;
	list_of(SndBuildInfo) snd_infos;
} PathList;

extern PathList scan_for_files();

typedef struct {
	list_of(str) include_paths;
    list_of(str) lib_paths;
    str lib_file;
	str html_src_path; // Stores the "basic_shell.html" used for web builds

	str emcc;
} Config;

void save_path_list(PathList *list);
void make_ninja_file(PathList *list, Config *config, bool is_release, bool is_web);
PathList load_path_list();

Config load_config();

// Directory tree
void ensure_directory_tree_exists();

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

    int spritesheet_index;

    struct {
        uint16_t sx;
        uint16_t sy;
        uint16_t ex;
        uint16_t ey;
    } coords;
} TexFileInfo;

typedef struct {
	str name;
	int frame_count;

    int index_first;
    int index_last;
} AnimInfo;



void load_tex_file(const char *path, list_of(TexFileInfo) *output, list_of(AnimInfo) *anim_output);
TexBuildInfo load_tex_build_info(const char *path);
SndBuildInfo load_snd_build_info(const char *path);

typedef struct {
	str name;
	str type;
	str parent;
	list_of(str) initializers;
} PonyFileTreeEntry;

typedef struct {
	str file_path;
	str type_name;
	str type_base_class;
	list_of(str) struct_fields;
	list_of(str) header_lines;
	list_of(PonyFileTreeEntry) tree_entries;

	bool has_tick;
	bool has_construct;
	bool has_destruct;
	bool has_tree;

	str tick_func;
	str construct_func;
	str destruct_func;
} PonyFileInfo;

#define DIR_TREE_UNKNOWN 0
#define DIR_TREE_DIRECTORY 1
#define DIR_TREE_TEX 2
#define DIR_TREE_SND 3
#define DIR_TREE_PONY 4

PonyFileInfo load_pony_file(const char *path);

typedef struct DirTree {
    list_of(struct DirTree) children;
    str name;

    int type;

    list_of(TexFileInfo) tex_infos;
    list_of(AnimInfo) anim_infos;

	SndBuildInfo snd_info;

	// This is stored as a pointer so that it will have a unique address.
	// TODO: Free this memory when we're done with it. (or don't, if we don't care.)
	PonyFileInfo *pony_info;
} DirTree;

typedef struct {
	// This is weird... we basically want the canonoical value of each of these
	// lists to be in the DirTree's, but those are kind of used as value types.
	//
	// This should be fine though.
    list_of(TexFileInfo*) tex_list;
    list_of(AnimInfo*)    anim_list;

	list_of(PonyFileInfo*) pony_list;

    PathList path_list;
    Config config;

    DirTree tree;

    int spritesheet_count;

	int anim_frame_ptr;
} ProjectFiles;

void get_dir_tree(ProjectFiles *output);

// Image processing and resource processing
void pack_images(ProjectFiles *pf);
void build_resource_loader(ProjectFiles *output);
void build_resource_header(ProjectFiles *output);
void rebuild_resources(bool pack);

#define FILE_NAME_RES_H ".ponygame/my.res.h"
#define FILE_NAME_RES_LOADER ".ponygame/res_loader.c"

// Process aseprite
void process_aseprite_from_tex(const char*);

void generate_file_res_debug_c();
void generate_file_res_release_c(ProjectFiles *pf);

void generate_file_pony_source_c(ProjectFiles *pf);
void generate_file_my_ponygame_h(ProjectFiles *pf);