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
	mat4 projection_sprites;

	int32_t screen_width;
	int32_t screen_height;

	int32_t frame_width;
	int32_t frame_height;

	// The data used to compute the screen scaling, etc.
	struct {
		// How big the screen should be, at least.
		int32_t target_width;
		int32_t target_height;

		// The currently computed scale factor, e.g. x1, x2, x3
		float scale_f;

		// The offset to center 0,0 on the screen.
		vec2 offset;

		// How much the screen is shifted off the real screen.
		int32_t computed_x_offset;
		int32_t computed_y_offset;
	} screen;
} RenderContext;

extern RenderContext ctx;