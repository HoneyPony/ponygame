#include "pony_render.h"

#include "pony_fs.h"
#include "pony_log.h"
#include "pony_opengl.h"

#include <stdio.h>

GLuint gltex_load(const char *path) {
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	// Nearest neighbor interpolation for pixel art
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Clamp colors, should prevent any wrap color glitches
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	FSImg image;
	image = fs_load_png(path, true);

	if(image.data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height,
			0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
		logf_verbose("successfully loaded GL texture %s", path);
	}

	fs_free_img_data(&image);

	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;
}