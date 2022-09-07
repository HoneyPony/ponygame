#include <stdio.h>
#include <stdlib.h>

#include "render.h"

// Declare the global context.
RenderContext ctx;

void render_init() {
#ifndef __EMSCRIPTEN__
	GLenum err = glewInit();
	if(err != GLEW_OK) {
		logf_error("GLEW error: %s", glewGetErrorString(err));
		exit(-1);
	}
#endif

	ctx.frame_width = 480; // TODO: Do initial computation with window size...?
	ctx.frame_height = 360;

	// TODO: REMOVE THIS!!! TEST CODE!!!

	sprite_test_tex.texture = gltex_load("res://test_sprite.png");
	sprite_test_tex.bottom_left_uv = vxy(0.0, 0.0);
	sprite_test_tex.bottom_right_uv = vxy(1.0, 0.0);
	sprite_test_tex.top_left_uv = vxy(0.0, 1.0);
	sprite_test_tex.top_right_uv = vxy(1.0, 1.0);
	sprite_test_tex.px_size = vxy(16.0, 16.0);

	// END TEST CODE

	render_init_framebuffer();
	render_init_objects();
	render_init_sprite();
}

// TODO: REMOVE THIS!!! TEST CODE!!!
TexHandle sprite_test_tex;
// END TEST CODE

void render_game_objects() {
	glBindFramebuffer(GL_FRAMEBUFFER, ctx.pixel_fb.framebuffer);
	glViewport(0, 0, ctx.frame_width, ctx.frame_height);

	glClearColor(0.3, 0.3, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	render_objects();
}

void render() {
	render_refresh_sprite();

	render_game_objects();
	render_framebuffer();	
}