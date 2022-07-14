#include "pony_render.h"

#include "pony_opengl.h"

#include <stdio.h>
#include <stdlib.h>

#include "pony_log.h"
#include "shader_src.h"

typedef struct {
	// The framebuffer used for all pixel graphics operations. This
	// framebuffer is rendered with nearest neighbor filtering to display
	// pixel-perfect art on screen.
	GLuint pix_framebuffer;
	GLuint pix_tex;
	GLuint pix_depth_stencil_rbo;

	
} RenderCtx;

struct {
	GLuint shader;
	GLuint mat;
	GLuint tex;
} sprite_shader;

struct {
	GLuint shader;
	GLuint tex;
} frame_shader;

static mat4 projection;


static RenderCtx ctx;

static GLuint tex;


float vertices[] = {
	1.f,  1.f,  1.f,    0.f, 0.f,
	1.f,  17.f, 1.f,    0.f, 1.f,
	17.f, 17.f, 1.f,    1.f, 1.f,

	17.f, 17.f, 1.f,    1.f, 1.f,
	1.f,  1.f,  1.f,    0.f, 0.f,
	17.f, 1.f,  1.f,    1.f, 0.f
};

float fb_verts[] = {
	-1.f, -1.f,    0.f, 0.f,
	-1.f,  1.f,    0.f, 1.f,
	 1.f,  1.f,    1.f, 1.f,

	 1.f,  1.f,    1.f, 1.f,
	-1.f, -1.f,    0.f, 0.f,
	 1.f, -1.f,    1.f, 0.f
};

static GLuint vbo;

static GLuint fb_vbo;

static int screen_width, screen_height;
static int frame_width, frame_height; // TODO: Consolidate all variables in one place

static void init_ctx() {
	glGenFramebuffers(1, &ctx.pix_framebuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, ctx.pix_framebuffer);

	glGenTextures(1, &ctx.pix_tex);
	glBindTexture(GL_TEXTURE_2D, ctx.pix_tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 480, 360, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// WebGL textures that are non-power-of-two must clamp to edge.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctx.pix_tex, 0);

	glGenRenderbuffers(1, &ctx.pix_depth_stencil_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, ctx.pix_depth_stencil_rbo);
	// GLES2: Very limited render buffer format selection. GL_DEPTH_COMPONENT16 is the only depth one available.
	// We could consider using GLES3, maybe, for better selection...?
	// Could also consider using a texture instead of a renderbuffer, but I would have
	// to read into that to see if it would even have more options at all.
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 480, 360);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ctx.pix_depth_stencil_rbo);

	int status;
	if((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
		logf_error("framebuffer status incomplete: %x", status);
	}
}

void resize_framebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, ctx.pix_framebuffer);

	glBindTexture(GL_TEXTURE_2D, ctx.pix_tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctx.pix_tex, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, ctx.pix_depth_stencil_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, frame_width, frame_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ctx.pix_depth_stencil_rbo);
}

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

	frame_width = 480; // TODO: Do initial computation with window size...?
	frame_width = 360;

	//glGenVertexArrays(1, &vao);
	//glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &fb_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, fb_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fb_verts), fb_verts, GL_STATIC_DRAW);

	init_ctx();
	tex = gltex_load("res://test_sprite.png");
}

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

	float scale_x = (float)(frame_width * 4) / (float)(screen_width);
	float scale_y = (float)(frame_height * 4) / (float)(screen_height);

	scale_x = 1.0 + ((scale_x - 1.0) / 2.0);
	scale_y = 1.0 + ((scale_y - 1.0) / 2.0);

	logf_info("width: %d vs %d", frame_width * 4, screen_width);

	float posx = scale_x;
	float negx = -scale_x;
	float posy = scale_y;
	float negy = -scale_y;

	// First triangle
	fb_verts[0] = negx;
	fb_verts[1] = negy;

	fb_verts[4] = negx;
	fb_verts[5] = posy;

	fb_verts[8] = posx;
	fb_verts[9] = posy;

	logf_info("x scale, y scale: %f %f", scale_x, scale_y);

	// Second triangle
	fb_verts[12] = posx;
	fb_verts[13] = posy;

	fb_verts[16] = negx;
	fb_verts[17] = negy;

	fb_verts[20] = posx;
	fb_verts[21] = negy;

	// Upload buffer
	glBindBuffer(GL_ARRAY_BUFFER, fb_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fb_verts), fb_verts, GL_STATIC_DRAW);
}

void render_fit_window(int width, int height) {
	if(width == screen_width && height == screen_height) return;

	screen_width = width;
	screen_height = height;


	frame_width = (width + 3) / 4; // Need to effectively take ceiling of these
	                               // dimensions, due to compute_screen_vertices, etc
	frame_height = (height + 3) / 4;

	resize_framebuffer();
	compute_screen_vertices();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glViewport(0, 0, width, height);

	float x = frame_width;//width;
	float y = frame_height;//height;

	mat4_ortho(&projection,
		0, x,
		0, y,
		-10, 10); // TODO: Determine Z range
}

void render() {
	

	glBindFramebuffer(GL_FRAMEBUFFER, ctx.pix_framebuffer);
	glViewport(0, 0, frame_width, frame_height);
	//glViewport(0, 0, 480, 360);


	glClearColor(0.3, 0.3, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(sprite_shader.shader);

	shader_set_mat4(sprite_shader.mat, &projection);
	shader_set_int(sprite_shader.tex, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// TODO: Implement this more broadly
	// Details: it appears that the minimal subset of WebGL supported by
	// Emscripten does not support vertex array objects, e.g. glBindVertexArray
	//
	// We could use WebGL 2.0 as I understand, but I think it might make more sense
	// to stick with a minimal subset for now, as the renderer is not going to be
	// doing that much manipulation of the attributes anyways.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); 

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	
	
	//glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screen_width, screen_height);

	glUseProgram(frame_shader.shader);

	shader_set_int(frame_shader.tex, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ctx.pix_tex);

	glBindBuffer(GL_ARRAY_BUFFER, fb_vbo);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	

	
	glDrawArrays(GL_TRIANGLES, 0, 6);
}