#pragma once

#include "pony_opengl.h"
#include "pony_glm.h"

typedef struct {
	// This data is associated with the framebuffer used to render most game
	// content to. It is set to the exact pixel size of the game, which is then
	// scaled up with nearest neighbor filtering to fill the screen.
	struct {
		GLuint framebuffer;
		GLuint color_tex;
		GLuint depth_stencil_rbo;

		GLuint rect_vbo;
	} pixel_fb;

	// Data associated with the sprite renderer: the VBO and the EBO.
	struct {
		GLuint vbo;
		GLuint ebo;
	} sprite_render;

	mat4 projection;

	int32_t screen_width;
	int32_t screen_height;

	int32_t frame_width;
	int32_t frame_height;
} RenderContext;

extern RenderContext ctx;