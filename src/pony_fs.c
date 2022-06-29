#include "pony_fs.h"

#include <string.h>

#define STBI_ONLY_PNG // Only need PNG support for now...
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "pony_string.h"

FSImg fs_load_png(const char *path) {
	if(cstr_has_prefix(path, "res://")) {
		/* TODO: Implement loading from memory in final builds */

		// For debug builds: remove the prefix, use the working directory to
		// load resources
		path += 6;
	}

	FSImg result = { 0 };

	int width, height;
	uint8_t *data = stbi_load(path, &width, &height, NULL, 4);

	if(!data) {
		printf("[ponygame] failed to load image %s\n", path);
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