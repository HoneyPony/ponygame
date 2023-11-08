#ifndef PONY_NUKLEAR_H
#define PONY_NUKLEAR_H

#define PONYGAME_NK_OPENGLES2

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_BOOL
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_IO
#include "../vendor/Nuklear/nuklear.h"

#define PONY_NK(...) __VA_ARGS__

#if defined(PONYGAME_NK_OPENGLES2)
	#ifdef PONY_NK_BACKEND_IMPLEMENTATION
		#define NK_SDL_GLES2_IMPLEMENTATION
	#endif
	#include "../vendor/Nuklear/demo/sdl_opengles2/nuklear_sdl_gles2.h"
#elif defined(PONYGAME_NK_OPENGL3)
	#ifdef PONY_NK_BACKEND_IMPLEMENTATION
		#define NK_SDL_GL3_IMPLEMENTATION
	#endif
	#include "../vendor/Nuklear/demo/sdl_opengl3/nuklear_sdl_gl3.h"
#else
	#ifdef PONY_NK_BACKEND_IMPLEMENTATION
		#define NK_SDL_GL2_IMPLEMENTATION
	#endif
	#include "../vendor/Nuklear/demo/sdl_opengl2/nuklear_sdl_gl2.h"
#endif

extern void pony_ui(struct nk_context *ctx, int32_t win_width, int32_t win_height);
extern void pony_fonts(struct nk_font_atlas *atlas);

#endif