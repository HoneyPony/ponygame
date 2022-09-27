#include "pony_input.h"
#include "pony_clib.h"

#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_keycode.h>

#include <SDL2/SDL.h>

static InputButton **scancode_to_key_map;
static int scancode_key_count;

// Define input variables
struct PonyKeyboard keys;

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
}

void pony_update_input_post() {
	for(int i = 0; i < scancode_key_count; ++i) {
		InputButton *button = scancode_to_key_map[i];
		button->was_pressed = button->pressed;
	}
}