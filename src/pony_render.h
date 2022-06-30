#pragma once

#include "pony_gl.h"
#include "pony_glm.h"

extern void render_init();
extern void render();

extern GLuint shader_compile(const char *vert_src, const char *frag_src);
extern GLuint shader_name(GLuint shader, const char *name);

extern void shader_bind(GLuint shader);
extern void shader_set_mat4(GLuint var, mat4 *mat);
extern void shader_set_int(GLuint var, int what);

extern GLuint gltex_load(const char *path);