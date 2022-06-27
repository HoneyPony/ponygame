#pragma once

#include "pony_gl.h"

extern void render_init();
extern void render();

extern GLuint shader_compile(const char *vert_src, const char *frag_src);