#include "pony_render.h"

#include "pony_opengl.h"

#include <stdio.h>
#include <stdlib.h>

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
} framebuffer_shader;

static mat4 projection;


static RenderCtx ctx;

static GLuint tex;


float vertices[] = {
	0.f,  0.f,  0.f,    0.f, 0.f,
	0.f,  10.f, 0.f,    0.f, 1.f,
	10.f, 10.f, 0.f,    1.f, 1.f,

	10.f, 10.f, 0.f,    1.f, 1.f,
	0.f,  0.f,  0.f,    0.f, 0.f,
	10.f, 0.f,  0.f,    1.f, 0.f
};

static GLuint vao, vbo;


static void init_ctx() {
	glGenFramebuffers(1, &ctx.pix_framebuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, ctx.pix_framebuffer);

	glGenTextures(1, &ctx.pix_tex);
	glBindTexture(GL_TEXTURE_2D, ctx.pix_tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 480, 360, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctx.pix_tex, 0);

	glGenRenderbuffers(1, &ctx.pix_depth_stencil_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, ctx.pix_depth_stencil_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 480, 360);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ctx.pix_depth_stencil_rbo);
}

void render_init() {
#ifndef __EMSCRIPTEN__
	GLenum err = glewInit();
	if(err != GLEW_OK) {
		printf("[ponygame] GLEW error: %s\n", glewGetErrorString(err));
		exit(-1);
	}
#endif

	sprite_shader.shader = shader_compile(shader_src_sprite_vert, shader_src_sprite_frag);
	sprite_shader.mat = shader_name(sprite_shader.shader, "transform");
	sprite_shader.tex = shader_name(sprite_shader.shader, "tex");

	//glGenVertexArrays(1, &vao);
	//glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	

	mat4_ortho(&projection,
		-20, 20,
		-20, 20,
		-10, 10);
	//init_ctx();
	tex = gltex_load("res://test_sprite.png");
}

void render() {
	glUseProgram(sprite_shader.shader);

	

	shader_set_mat4(sprite_shader.mat, &projection);
	shader_set_int(sprite_shader.tex, 0);

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
}