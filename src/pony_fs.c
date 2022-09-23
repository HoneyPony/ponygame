#include "pony_fs.h"

#include <string.h>

#define STBI_ONLY_PNG // Only need PNG support for now...
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "pony_log.h"
#include "pony_string.h"

FSImg fs_load_png(const char *path, bool flip) {
	int width, height;
	uint8_t *data;

	stbi_set_flip_vertically_on_load(flip);

	if(cstr_has_prefix(path, "res://")) {
		if(pony_resources_are_packed) {
			FSPackedMem mem = fs_find_packed_resource(path + 6);
			data = stbi_load_from_memory(mem.ptr, mem.length, &width, &height, NULL, 4);
		}
		else {
			// For debug builds: remove the prefix, use the working directory to
			// load resources
			logf_info("debug: loading %s", path + 6);
			data = stbi_load(path + 6, &width, &height, NULL, 4);
		}
	}
	else {
		data = stbi_load(path, &width, &height, NULL, 4);
	}

	FSImg result = { 0 };

	if(!data) {
		logf_error("failed to load image %s", path);
	}

	result.data = data;
	result.width = width;
	result.height = height;

	return result;
}

void fs_free_img_data(FSImg *ptr) {
	if(ptr->data) {
		stbi_image_free(ptr->data);
		ptr->data = NULL;
	}
	ptr->width = 0;
	ptr->height = 0;
}