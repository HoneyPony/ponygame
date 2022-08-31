#pragma once

#include "pony_gl.h"
#include "pony_glm.h"
#include "pony_node.h"

extern void render_init();
extern void render_fit_window(int width, int height);
extern void render();

extern GLuint shader_compile(const char *vert_src, const char *frag_src);
extern GLuint shader_name(GLuint shader, const char *name);

extern void shader_bind(GLuint shader);
extern void shader_set_mat4(GLuint var, mat4 *mat);
extern void shader_set_int(GLuint var, int what);

extern GLuint gltex_load(const char *path);

typedef struct {
	GLuint texture;

	vec2 uv_bottom_left;
	vec2 uv_top_right;

	// Used to determine how the texture should be centered around a pivot point.
	vec2 px_size;
} TexHandle;

typedef struct {
	Node *source;
	TexHandle *tex;
	vec2 pivot;
} TexRenderer;

/**
 * Pushes a new render command that renders a rectangular texture at the given
 * node. The texture is rendered with the node transform. The specific pivot
 * for texture must also be specified.
 * 
 * @param node 
 * @param tex 
 * @param tex_pivot 
 */
extern void render_tex_on_node(Node *node, TexHandle *tex, vec2 tex_pivot, bool snap);

// TODO: REMOVE THIS!!! TEST CODE!!!
extern TexHandle sprite_test_tex;