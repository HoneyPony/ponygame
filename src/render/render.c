#include <stdio.h>
#include <stdlib.h>

#include "pony_render.h"

#include "pony_opengl.h"

#include "pony_log.h"
#include "shader_src.h"

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

	GLuint sprite_vbo;

	mat4 projection;

	int32_t screen_width;
	int32_t screen_height;

	int32_t frame_width;
	int32_t frame_height;
} RenderContext;

struct {
	GLuint shader;
	GLuint mat;
	GLuint tex;
} sprite_shader;

struct {
	GLuint shader;
	GLuint tex;
} frame_shader;

static RenderContext ctx;

float vertices[] = {
	1.f,  1.f,  1.f,    0.f, 0.f,
	1.f,  17.f, 1.f,    0.f, 1.f,
	17.f, 17.f, 1.f,    1.f, 1.f,

	17.f, 17.f, 1.f,    1.f, 1.f,
	1.f,  1.f,  1.f,    0.f, 0.f,
	17.f, 1.f,  1.f,    1.f, 0.f
};

float pixel_fb_verts[] = {
	-1.f, -1.f,    0.f, 0.f,
	-1.f,  1.f,    0.f, 1.f,
	 1.f,  1.f,    1.f, 1.f,

	 1.f,  1.f,    1.f, 1.f,
	-1.f, -1.f,    0.f, 0.f,
	 1.f, -1.f,    1.f, 0.f
};

static GLuint vbo;
static GLuint ebo;

static void init_ctx() {
	glGenFramebuffers(1, &ctx.pixel_fb.framebuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, ctx.pixel_fb.framebuffer);

	glGenTextures(1, &ctx.pixel_fb.color_tex);
	glBindTexture(GL_TEXTURE_2D, ctx.pixel_fb.color_tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 480, 360, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// WebGL textures that are non-power-of-two must clamp to edge.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctx.pixel_fb.color_tex, 0);

	glGenRenderbuffers(1, &ctx.pixel_fb.depth_stencil_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, ctx.pixel_fb.depth_stencil_rbo);
	// GLES2: Very limited render buffer format selection. GL_DEPTH_COMPONENT16 is the only depth one available.
	// We could consider using GLES3, maybe, for better selection...?
	// Could also consider using a texture instead of a renderbuffer, but I would have
	// to read into that to see if it would even have more options at all.
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 480, 360);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ctx.pixel_fb.depth_stencil_rbo);

	int status;
	if((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
		logf_error("framebuffer status incomplete: %x", status);
	}
}

void resize_framebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, ctx.pixel_fb.framebuffer);

	glBindTexture(GL_TEXTURE_2D, ctx.pixel_fb.color_tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ctx.frame_width, ctx.frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctx.pixel_fb.color_tex, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, ctx.pixel_fb.depth_stencil_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ctx.frame_width, ctx.frame_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ctx.pixel_fb.depth_stencil_rbo);
}

// TODO: Properly modularize renderer.
extern void render_init_lists();
extern void render_lists(GLuint vbo, GLuint ebo);

void render_init() {
#ifndef __EMSCRIPTEN__
	GLenum err = glewInit();
	if(err != GLEW_OK) {
		logf_error("GLEW error: %s", glewGetErrorString(err));
		exit(-1);
	}
#endif

	sprite_shader.shader = shader_compile(shader_src_sprite_vert, shader_src_sprite_frag);
	sprite_shader.mat = shader_name(sprite_shader.shader, "transform");
	sprite_shader.tex = shader_name(sprite_shader.shader, "tex");

	frame_shader.shader = shader_compile(shader_src_frame_vert, shader_src_frame_frag);
	frame_shader.tex = shader_name(frame_shader.shader, "texture");

	ctx.frame_width = 480; // TODO: Do initial computation with window size...?
	ctx.frame_height = 360;

	//glGenVertexArrays(1, &vao);
	//glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	//glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ctx.pixel_fb.rect_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, ctx.pixel_fb.rect_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pixel_fb_verts), pixel_fb_verts, GL_STATIC_DRAW);

	init_ctx();

	// TODO: REMOVE THIS!!! TEST CODE!!!

	sprite_test_tex.texture = gltex_load("res://test_sprite.png");
	sprite_test_tex.bottom_left_uv = vxy(0.0, 0.0);
	sprite_test_tex.bottom_right_uv = vxy(1.0, 0.0);
	sprite_test_tex.top_left_uv = vxy(0.0, 1.0);
	sprite_test_tex.top_right_uv = vxy(1.0, 1.0);
	sprite_test_tex.px_size = vxy(16.0, 16.0);

	// END TEST CODE

	render_init_lists();
}

// TODO: REMOVE THIS!!! TEST CODE!!!
TexHandle sprite_test_tex;
// END TEST CODE

void compute_screen_vertices() {
	// The idea here took some tinkering to nail down.
	// Essentially, what we want to do is scale up the pixel-perfect framebuffer
	// to cover the screen. We have previously ensured that the scaled framebuffer
	// will be as big or bigger than the screen.
	//
	// So, first, we compute how many pixels the scaled up framebuffer "overextends"
	// the real screen. Then, we can scale the vertices in the vertex buffer
	// so that they match this overextension.
	// 
	// In particular, we can divide the "overextended"

	float scale_x = (float)(ctx.frame_width * 4) / (float)(ctx.screen_width);
	float scale_y = (float)(ctx.frame_height * 4) / (float)(ctx.screen_height);

	scale_x = 1.0 + ((scale_x - 1.0) / 2.0);
	scale_y = 1.0 + ((scale_y - 1.0) / 2.0);

	logf_info("width: %d vs %d", ctx.frame_width * 4, ctx.screen_width);

	float posx = scale_x;
	float negx = -scale_x;
	float posy = scale_y;
	float negy = -scale_y;

	// First triangle
	pixel_fb_verts[0] = negx;
	pixel_fb_verts[1] = negy;

	pixel_fb_verts[4] = negx;
	pixel_fb_verts[5] = posy;

	pixel_fb_verts[8] = posx;
	pixel_fb_verts[9] = posy;

	logf_info("x scale, y scale: %f %f", scale_x, scale_y);

	// Second triangle
	pixel_fb_verts[12] = posx;
	pixel_fb_verts[13] = posy;

	pixel_fb_verts[16] = negx;
	pixel_fb_verts[17] = negy;

	pixel_fb_verts[20] = posx;
	pixel_fb_verts[21] = negy;

	// Upload buffer
	glBindBuffer(GL_ARRAY_BUFFER, ctx.pixel_fb.rect_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pixel_fb_verts), pixel_fb_verts, GL_STATIC_DRAW);
}

void render_fit_window(int width, int height) {
	if(width == ctx.screen_width && height == ctx.screen_height) return;

	ctx.screen_width = width;
	ctx.screen_height = height;


	ctx.frame_width = (width + 3) / 4; // Need to effectively take ceiling of these
	                               // dimensions, due to compute_screen_vertices, etc
	ctx.frame_height = (height + 3) / 4;

	resize_framebuffer();
	compute_screen_vertices();

	float x = ctx.frame_width;
	float y = ctx.frame_height;

	mat4_ortho(&ctx.projection,
		0, x,
		0, y,
		-10, 10); // TODO: Determine Z range
}


void render_game_objects() {
	glBindFramebuffer(GL_FRAMEBUFFER, ctx.pixel_fb.framebuffer);
	glViewport(0, 0, ctx.frame_width, ctx.frame_height);

	glClearColor(0.3, 0.3, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(sprite_shader.shader);

	shader_set_mat4(sprite_shader.mat, &ctx.projection);
	shader_set_int(sprite_shader.tex, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	render_lists(vbo, ebo);
}

void render_framebuffer_to_screen() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, ctx.screen_width, ctx.screen_height);

	glUseProgram(frame_shader.shader);

	shader_set_int(frame_shader.tex, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ctx.pixel_fb.color_tex);

	glBindBuffer(GL_ARRAY_BUFFER, ctx.pixel_fb.rect_vbo);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render() {
	render_game_objects();
	render_framebuffer_to_screen();	
}