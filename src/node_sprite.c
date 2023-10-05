#include "node_sprite.h"

#include "color.h"

void
sprite_play(Sprite *sprite, AnimHandle *handle) {
	sprite->accumulator = 0;
	sprite->current_animation = handle;
}

void
sprite_set_hsv(Sprite *sprite, float hsv[3]) {
	float rgb[3];
	hsv_to_rgb(hsv, rgb);

	sprite->r = rgb[0];
	sprite->g = rgb[1];
	sprite->b = rgb[2];
}

void
sprite_get_hsv(Sprite *sprite, float hsv[3]) {
	float rgb[3];
	rgb[0] = sprite->r;
	rgb[1] = sprite->g;
	rgb[2] = sprite->b;

	rgb_to_hsv(rgb, hsv);
}