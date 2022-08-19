#ifndef __EMSCRIPTEN__
	#define GLEW_STATIC
	#include <GL/glew.h>
#else
	#include <GL/gl.h>

	#include <GLES2/gl2.h>
	#include <EGL/egl.h>
#endif