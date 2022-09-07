#include "render.h"

struct {
	GLuint shader;
	GLuint tex;
} frame_shader;

float pixel_fb_verts[] = {
	-1.f, -1.f,    0.f, 0.f,
	-1.f,  1.f,    0.f, 1.f,
	 1.f,  1.f,    1.f, 1.f,

	 1.f,  1.f,    1.f, 1.f,
	-1.f, -1.f,    0.f, 0.f,
	 1.f, -1.f,    1.f, 0.f
};

static void resize_framebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, ctx.pixel_fb.framebuffer);

	glBindTexture(GL_TEXTURE_2D, ctx.pixel_fb.color_tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ctx.frame_width, ctx.frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctx.pixel_fb.color_tex, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, ctx.pixel_fb.depth_stencil_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ctx.frame_width, ctx.frame_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ctx.pixel_fb.depth_stencil_rbo);
}

static void compute_screen_vertices() {
	// The idea here took some tinkering to nail down.
	// Essentially, what we want to do is scale up the pixel-perfect framebuffer
	// to cover the screen. We have previously ensured that the scaled framebuffer
	// will be as big or bigger than the screen.
	//
	// So, first, we compute how many pixels the scaled up framebuffer "overextends"
	// the real screen. Then, we can scale the vertices in the vertex buffer
	// so that they match this overextension.
	// 
	// I'm haven't yet figured out how to explain the factor of two needed.

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

// The framebuffer renderer is basically responsible for the screen, so it
// implements this function for now.
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

void render_init_framebuffer() {
	// Compile the framebuffer shader
	frame_shader.shader = shader_compile(shader_src_frame_vert, shader_src_frame_frag);
	frame_shader.tex = shader_name(frame_shader.shader, "texture");

	// Generate the VBO for drawing the framebuffer.
	glGenBuffers(1, &ctx.pixel_fb.rect_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, ctx.pixel_fb.rect_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pixel_fb_verts), pixel_fb_verts, GL_STATIC_DRAW);

	// Setup the framebuffer object itself, with a texture.
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

void render_framebuffer() {
	// Bind the default framebuffer, in order to render to screen.
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