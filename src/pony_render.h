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

	vec2 bottom_left_uv;
	vec2 bottom_right_uv;
	vec2 top_left_uv;
	vec2 top_right_uv;

	// Used to determine how the texture should be centered around a pivot point.
	vec2 px_size;
} TexHandle;

typedef enum {
	SNAP_NONE = 2,
	SNAP_EVEN = 0,
	SNAP_ODD = 1
} SnapType;

extern SnapType snap_for_dimension(float dim);
extern float snap_coordinate(float coord, SnapType type);

typedef struct {
	Node *node;
	Node *snap_relative;
	TexHandle *tex;
	vec2 tex_pivot;
	SnapType snap_x;
	SnapType snap_y;

	float r;
	float g;
	float b;
	float a;

	bool on_camera;
} TexRenderer;

typedef struct {
    TexHandle texture;
    int time_ms;
} AnimFrame;

typedef struct {
    size_t frame_count;
    AnimFrame *frames;
} AnimHandle;

/**
 * Pushes a new render command that renders a rectangular texture at the given
 * node. The texture is rendered with the node transform. The specific pivot
 * for texture must also be specified.
 */
extern void render_tex_on_node(TexRenderer renderer);

/**
 * @brief Stores the current position that the camera is centered on.
 * Set this value in order to move the camera.
 */
extern vec2 camera_point;

extern float clear_color[4];