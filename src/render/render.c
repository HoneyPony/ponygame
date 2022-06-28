#include "pony_render.h"

#ifndef __EMSCRIPTEN__
	#include <GL/glew.h>
#else
	#include <GL/gl.h>

	#include <GLES2/gl2.h>
	#include <EGL/egl.h>
#endif

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


static RenderCtx ctx;

static GLuint shader;

float vertices[] = {
	-0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
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

	shader = shader_compile(shader_src_sprite_vert, shader_src_sprite_frag);

	//glGenVertexArrays(1, &vao);
	//glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); 

	//init_ctx();

}

void render() {
	glUseProgram(shader);

	// TODO: Implement this more broadly
	// Details: it appears that the minimal subset of WebGL supported by
	// Emscripten does not support vertex array objects, e.g. glBindVertexArray
	//
	// We could use WebGL 2.0 as I understand, but I think it might make more sense
	// to stick with a minimal subset for now, as the renderer is not going to be
	// doing that much manipulation of the attributes anyways.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); 
	//glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}