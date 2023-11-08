#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <stdio.h>
#include "pony_clib.h"

#ifdef __EMSCRIPTEN__
	#include <emscripten.h>

	EM_JS(int, js_get_window_width, (), { return window.innerWidth; });
	EM_JS(int, js_get_window_height, (), { return window.innerHeight; });

	static bool has_setup_main_loop_timing = false;
#endif

#include "pony_internal.h"
#include "pony_render.h"
#include "pony_log.h"
#include "pony_compiler_features.h"
#include "pony_node.h"
#include "pony.main.h"
#include "pony_input.h"

/* Basic UI support; TODO our own UI library? */
#include "ponygame_nuklear.h"

/* Do we want multiple window support? */
static SDL_Window *pony_main_window = NULL;
static SDL_GLContext pony_main_context;
static struct nk_context *pony_main_nk_ctx;

static uint32_t is_vsync = 0;

static void pony_render() {
	//puts("rendering");
	//int width, height;
	//SDL_GetWindowSize(pony_main_window, &width, &height);
#ifdef __EMSCRIPTEN__
	//int ww = get_canvas_width();
	int js_width = js_get_window_width();
	int js_height = js_get_window_height();
	SDL_SetWindowSize(pony_main_window, js_width, js_height);
#endif

	int width, height;
	SDL_GetWindowSize(pony_main_window, &width, &height);

	
	render_fit_window(width, height);
	render();

	/* Render the nuklear UI */
	glViewport(0, 0, width, height);
	//glClearColor(0.3, 0.3, 0.3, 1.0);
	//glClear(GL_COLOR_BUFFER_BIT);

	pony_ui(pony_main_nk_ctx, width, height);
	
	nk_sdl_render(NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
}

/* TODO: Handle this by closing event loop */
static void pony_quit() {
	exit(0);
}

double total_frame_time = 0.0;
uint64_t frame_time_samples = 0;

static void pony_event_loop() {
// Annoyingly, the emscripten_set_main_loop function seems to never return.
// This means that there's really no way to call single one-shot functions after
// it besides using a flag.
#ifdef __EMSCRIPTEN__
	if(!has_setup_main_loop_timing) {
		int success = emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
		has_setup_main_loop_timing = true;
		//logf_info("setup main loop. success = %d", success);
	}
#endif

	uint64_t non_render_time = SDL_GetTicks64();
	SDL_Event evt;
	
	uint32_t frame_time = SDL_GetTicks();
	
	nk_input_begin(pony_main_nk_ctx);
	while(SDL_PollEvent(&evt)) {
		if(evt.type == SDL_QUIT) pony_quit();
		
		/* Handle other events */
		nk_sdl_handle_event(&evt);
	}
	nk_input_end(pony_main_nk_ctx);

	// Update input after processing events
	pony_update_input_pre();

	uint64_t something_time = SDL_GetTicks64() - non_render_time;

    pony_tick_start(); // User can define some code that happens around the pony loop
	node_process_all();
    pony_tick_end();

	// Update input state for next frame before rendering
	pony_update_input_post();

	non_render_time = SDL_GetTicks64() - non_render_time;
	uint64_t render_time = SDL_GetTicks64();
	pony_render();
	SDL_GL_SwapWindow(pony_main_window);
	render_time = SDL_GetTicks64() - render_time;

	//logf_info("frame breakdown: %dms process, %dms render [%dms event polling]", non_render_time, render_time, something_time);

	frame_time = (SDL_GetTicks() - frame_time);

	total_frame_time += frame_time;
	frame_time_samples += 1;

	if(frame_time_samples == 60) {
		double avg_frame_time = (total_frame_time / (double)frame_time_samples);
        int fps = (int)(1 / (avg_frame_time / 1000.0));
		logf_info("FPS: %d | Average frame time: %f ms", fps, avg_frame_time);

		total_frame_time = 0;
		frame_time_samples = 0;
	}
	
#ifndef __EMSCRIPTEN__
	// Perform manual FPS limiting if there is no vsync.
	if(!is_vsync && frame_time < 16) {
		uint32_t delay = 16 - frame_time;

		SDL_Delay(delay);
	}
#endif
}

extern void test();
extern void pony_load_resources();
extern void pony_init_user_nodes();

extern const char *game_title;

int main(UNUSED int argc, UNUSED char **argv) {
	logf_info("welcome to Untitled Game");

	pony_init_builtin_nodes();

	//test();

	//puts("[ponygame] game is complete");

	//return 0;

	if(SDL_Init(SDL_INIT_VIDEO)) {
		logf_error("unable to initialize SDL: %s", SDL_GetError());
		fflush(stdout);
		//puts("[ponygame] SDL_Init failure.");
		exit(-1);
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	
	pony_main_window = SDL_CreateWindow(game_title,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640,
		480,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_MAXIMIZED);

	logf_info("initialized window.");
		
	if(!pony_main_window) {
		logf_error("could not create SDL window.");
		exit(-1);
	}
	
	pony_main_context = SDL_GL_CreateContext(pony_main_window);
	
	if(!pony_main_context) {
		logf_error("could not create GL context.");
		exit(-1);
	}

	// Must initialize GLEW before nuklear.
	render_init();

	pony_main_nk_ctx = nk_sdl_init(pony_main_window);
	struct nk_font_atlas *atlas;
	nk_sdl_font_stash_begin(&atlas);
	pony_fonts(atlas);
	/* TODO: Build resource system (i.e. packing into binary) for fonts */
	nk_sdl_font_stash_end();

	pony_input_init();

	

	SDL_GL_SetSwapInterval(1);
	is_vsync = SDL_GL_GetSwapInterval();

	// Initialize sound before loading resources
	pony_init_sound();

	// Nodes must be initialized before resources are loaded so that the load_resources
	// function can override the associated_tree of the node headers.
	pony_init_user_nodes();

	pony_load_resources();

	logf_info("finished loading resources.");

    // Run user initialization code after all of the engine initialization code.
    pony_begin();

	logf_info("finished all initialization.");

#ifdef __EMSCRIPTEN__
	is_vsync = 1;

	emscripten_set_main_loop(pony_event_loop, 0, 1);

	return 0;
#else

	for(;;) {
		pony_event_loop(NULL);
	}
	
	nk_sdl_shutdown();
	SDL_GL_DeleteContext(pony_main_context);
	SDL_DestroyWindow(pony_main_window);
	SDL_Quit();
	
	return 0;
#endif
}