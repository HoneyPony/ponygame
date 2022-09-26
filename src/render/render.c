#include <stdio.h>
#include <stdlib.h>

#include "render.h"

// Declare the global context.
RenderContext ctx;

SnapType snap_for_dimension(float dim) {
	int x = (int)dim;
	if(x & 1) return SNAP_ODD;
	return SNAP_EVEN;
}

float snap_coordinate(float coord, SnapType type) {
	if(type == SNAP_EVEN) return round(coord);
	if(type == SNAP_ODD) return round(coord) + 0.5;
	return coord;
}

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

	render_init_framebuffer();
	render_init_objects();
	render_init_sprite();

	logf_info("generated GL buffers: [error = %d]", glGetError());
	logf_info("         framebuffer: %d", ctx.pixel_fb.rect_vbo);
	logf_info("       sprite render: %d", ctx.sprite_render.vbo);
	logf_info(" sprite render [ebo]: %d", ctx.sprite_render.ebo);
}

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