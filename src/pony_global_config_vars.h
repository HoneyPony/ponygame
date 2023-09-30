#ifndef PONY_GLOBAL_CONFIG_VARS_H
#define PONY_GLOBAL_CONFIG_VARS_H

extern bool pixel_art_game;

/* Whether the renderer can assume all commands are opaque (i.e. no blending / sorting needed)*/
extern bool may_assume_opaque;

extern const char* game_title;

extern bool disable_pixel_perfect_framebuffer;

#endif