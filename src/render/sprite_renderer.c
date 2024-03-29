#include "render.h"

struct {
	GLuint shader;
	GLuint mat;
	GLuint tex;
} sprite_shader;

void render_init_sprite() {
	sprite_shader.shader = shader_compile(shader_src_sprite_vert, shader_src_sprite_frag);
	sprite_shader.mat = shader_name(sprite_shader.shader, "transform");
	sprite_shader.tex = shader_name(sprite_shader.shader, "tex");

	glGenBuffers(1, &ctx.sprite_render.vbo);
	glGenBuffers(1, &ctx.sprite_render.ebo);
}

void render_refresh_sprite() {
	glUseProgram(sprite_shader.shader);

	shader_set_mat4(sprite_shader.mat, &ctx.projection_sprites);
	shader_set_int(sprite_shader.tex, 0);
}

void render_bind_sprite() {
	// Buffers
	glBindBuffer(GL_ARRAY_BUFFER, ctx.sprite_render.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx.sprite_render.ebo);

	// Attribute arrays
	// First attribute: Position: x, y, and z.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); 

	// Second attribute: UV's.
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Third attribute: colors.
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Shader
	glUseProgram(sprite_shader.shader);
}