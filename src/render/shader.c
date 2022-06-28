#include "pony_render.h"

#include "pony_clib.h"

#ifdef __EMSCRIPTEN__
	#include <GLES2/gl2.h>
	#include <EGL/egl.h>
#else
	#include <GL/glew.h>
#endif

#include <stdio.h>

static void print_shader_err(GLuint shader, const char *type) {
	int status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if(status == 0) {
		int length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		char *log = pony_malloc(length);
		glGetShaderInfoLog(shader, length, NULL, log);
		printf("[ponygame] ----- %s shader compile error: -----\n%s\n           -----------------------------------\n", type, log);

		pony_free(log);
	}
}

GLuint shader_compile(const char *vertex_src, const char *frag_src) {
	GLuint vert, frag;

	vert = glCreateShader(GL_VERTEX_SHADER);
	frag = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vert, 1, &vertex_src, NULL);
	glCompileShader(vert);

	print_shader_err(vert, "vertex");

	glShaderSource(frag, 1, &frag_src, NULL);
	glCompileShader(frag);

	print_shader_err(frag, "fragment");

	GLuint program = glCreateProgram();
	glAttachShader(program, vert);
	glAttachShader(program, frag);
	glLinkProgram(program);

	glDeleteShader(vert);
	glDeleteShader(frag);

	return program;
}