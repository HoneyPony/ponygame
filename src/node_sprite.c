#include "node_sprite.h"

void sprite_play(Sprite *sprite, AnimHandle *handle) {
	sprite->accumulator = 0;
	sprite->current_animation = handle;
}