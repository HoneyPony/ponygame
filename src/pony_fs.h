#pragma once

#include "pony_clib.h"

typedef struct {
	uint8_t *data;
	size_t width;
	size_t height;
} FSImg;

extern FSImg fs_load_png(const char *path, bool flip);
extern void fs_free_img_data(FSImg *ptr);