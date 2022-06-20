#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <stdio.h>

#ifdef __EMSCRIPTEN__
	#include <emscripten.h>
#endif

#include "pony_internal.h"

/* Do we want multiple window support? */
static SDL_Window *pony_main_window = NULL;
static SDL_GLContext pony_main_context;

static uint32_t is_vsync = 0;

static void pony_render() {
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_TRIANGLE_STRIP);

	glVertex2f(-0.5f, -0.5f);
	glVertex2f( 0.5f, -0.5f);
	glVertex2f(-0.5f,  0.5f);
	glVertex2f( 0.5f,  0.5f);
	
	glEnd();
}

/* TODO: Handle this by closing event loop */
static void pony_quit() {
	exit(0);
}

static void pony_event_loop(void *arg) {
	SDL_Event evt;
	
	uint32_t frame_time = SDL_GetTicks();
	
	while(SDL_PollEvent(&evt)) {
		if(evt.type == SDL_QUIT) pony_quit();
		
		/* Handle other events */
	}
	
	pony_render();
	SDL_GL_SwapWindow(pony_main_window);
	
	
	frame_time = (SDL_GetTicks() - frame_time);
	
	// Perform manual FPS limiting if there is no vsync.
	if(!is_vsync && frame_time < 16) {
		uint32_t delay = 16 - frame_time;

		SDL_Delay(delay);
	}
}

extern void test();

int main(int argc, char **argv) {
	puts("[ponygame] welcome to Untitled Game");

	pony_init_builtin_nodes();

	test();

	puts("[ponygame] game is complete");

	return 0;

	if(SDL_Init(SDL_INIT_VIDEO)) {
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		fflush(stdout);
		//puts("[ponygame] SDL_Init failure.");
		exit(-1);
	}
	
	pony_main_window = SDL_CreateWindow("Untitled Game",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640,
		480,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

	puts("[ponygame] initialized window.");
		
	if(!pony_main_window) {
		puts("[ponygame] Could not create SDL window.");
		exit(-1);
	}
	
	pony_main_context = SDL_GL_CreateContext(pony_main_window);
	
	if(!pony_main_context) {
		puts("[ponygame] Could not create GL context.");
		exit(-1);
	}
	
#ifdef __EMSCRIPTEN__
	is_vsync = 1;
	emscripten_set_main_loop_arg(pony_event_loop, NULL, 60, 1);

	return 0;
#else
	SDL_GL_SetSwapInterval(1);
	is_vsync = SDL_GL_GetSwapInterval();

	for(;;) {
		pony_event_loop(NULL);
	}
	
	SDL_GL_DeleteContext(pony_main_context);
	SDL_DestroyWindow(pony_main_window);
	SDL_Quit();
	
	return 0;
#endif
}