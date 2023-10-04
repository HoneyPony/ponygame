

#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_keycode.h>

#include <SDL2/SDL.h>

#include "pony_input.h"
#include "pony_clib.h"

#include "pony_render.h"
#include "render/render_context.h"

#include "pony_global_config_vars.h"

static InputButton **scancode_to_key_map;
static int scancode_key_count;

// Define input variables
struct PonyKeyboard keys;
struct PonyMouse mouse;

static InputButton dummy_input;

void pony_input_init() {
	SDL_GetKeyboardState(&scancode_key_count);

	scancode_to_key_map = malloc(sizeof(*scancode_to_key_map) * scancode_key_count);

	// Scancodes that do not map to any key (or, if we haven't bothered to define it yet)
	// should be mapped to a dummy memory location, so that the other code can just
	// loop through the whole list of scancodes without breaking anything.
	for(int i = 0; i < scancode_key_count; ++i) {
		scancode_to_key_map[i] = &dummy_input;
	}

	#define PONY_KEY(name, scancode)\
		scancode_to_key_map[scancode] = &keys.name;\
		*scancode_to_key_map[scancode] = (InputButton){0};

	#include "pony_input_xmacro.h"
	#undef PONY_KEY
}

void pony_update_input_pre() {
	// TODO: Do we want to check the numkeys from SDL again? It shouldn't change,
	// I don't think.
	const uint8_t *state = SDL_GetKeyboardState(NULL);
	for(int i = 0; i < scancode_key_count; ++i) {
		InputButton *button = scancode_to_key_map[i];
		button->pressed = state[i];
		button->just_pressed =   button->pressed && !button->was_pressed;
		button->just_released = !button->pressed &&  button->was_pressed;
	}

		uint32_t m = SDL_GetMouseState(NULL, NULL);

	mouse.left.pressed = !!(m & SDL_BUTTON_LMASK);
	mouse.right.pressed = !!(m & SDL_BUTTON_RMASK);

#define UPDATE(button)\
	button.just_pressed =   button.pressed && !button.was_pressed; \
	button.just_released = !button.pressed &&  button.was_pressed;

	UPDATE(mouse.left)
	UPDATE(mouse.right)
}

void pony_update_input_post() {
	for(int i = 0; i < scancode_key_count; ++i) {
		InputButton *button = scancode_to_key_map[i];
		button->was_pressed = button->pressed;
	}

	mouse.left.was_pressed = mouse.left.pressed;
	mouse.right.was_pressed = mouse.right.pressed;
}

vec2 mouse_screen() {
	int x, y;
	SDL_GetMouseState(&x, &y);

	y = ctx.screen_height - y;

	if(disable_pixel_perfect_framebuffer) {
		x -= ctx.screen_width / 2;
		y -= ctx.screen_height / 2;
	}
	else {
		x += ctx.screen.computed_x_offset;
		y += ctx.screen.computed_y_offset;
	}

	//logf_info("mouse: %d %d", x, y);

	float mx = (float)x / ctx.screen.scale_f;
	float my = (float)y / ctx.screen.scale_f;

	if(disable_pixel_perfect_framebuffer) {
		mx = (float)x * ctx.screen.scale_f;
		my = (float)y * ctx.screen.scale_f;

		//printf("scaled: %f %f | %f\n", mx, my, ctx.screen.scale_f);
	}

	return round(vxy(mx, my));
}

vec2 mouse_global() {
	return add(mouse_screen(), camera_point);
}