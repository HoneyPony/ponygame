#include "pony_render.h"

#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	// The framebuffer used for all pixel graphics operations. This
	// framebuffer is rendered with nearest neighbor filtering to display
	// pixel-perfect art on screen.
	GLuint pix_framebuffer;
	GLuint pix_tex;
	GLuint pix_depth_stencil_rbo;
} RenderCtx;


static RenderCtx ctx;

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
	GLenum err = glewInit();
	if(err != GLEW_OK) {
		printf("[ponygame] GLEW error: %s\n", glewGetErrorString(err));
		exit(-1);
	}

	init_ctx();
}

