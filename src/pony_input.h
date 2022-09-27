#pragma once

#include "pony_clib.h"

typedef struct {
	// Whether this button is currently pressed, on this frame.
	bool pressed;

	// Whether this button was pressed on the last frame, irrespective of whether
	// it is also pressed on this frame.
	bool was_pressed;

	// Whether this button was just pressed this frame, i.e. it was not pressed
	// last frame but is now.
	bool just_pressed;

	// Whether this button was just released this frame, i.e. it was pressed last
	// frame but it is not now.
	bool just_released;
} InputButton;

struct PonyKeyboard {
	#define PONY_KEY(keyname, sdl_scancode) InputButton keyname;
	#include "pony_input_xmacro.h"
	#undef PONY_KEY
};

extern struct PonyKeyboard keys;

extern void pony_input_init();
extern void pony_update_input_pre();
extern void pony_update_input_post();