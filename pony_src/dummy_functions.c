#include "pony_fs.h"

bool pony_resources_are_packed = false;

bool pixel_art_game = false; // Dummy config vars
bool may_assume_opaque = true;
bool disable_pixel_perfect_framebuffer = false;

FSPackedMem fs_find_packed_resource(const char *res_path) {
	FSPackedMem result = { 0 };
	return result;
}