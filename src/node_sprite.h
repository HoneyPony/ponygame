#pragma once

#include "pony_node.h"
#include "pony_render.h"

#define FieldList_Sprite \
FieldList_Node \
AnimHandle *current_animation; \
int current_frame; \
float accumulator; \
bool snap; \
bool on_camera; \
Node *snap_relative; \
float r; \
float g; \
float b; \
float a;

node_from_field_list(Sprite)

void sprite_play(Sprite *sprite, AnimHandle *anim);

/** Sets the color modulation of the given sprite in HSV. */
void sprite_set_hsv(Sprite *sprite, float hsv[3]);
/** Gets the color modulation of the given sprite in HSV. */
void sprite_get_hsv(Sprite *sprite, float hsv[3]);

#define FieldList_StaticSprite \
FieldList_Node \
TexHandle *texture; \
Node *snap_relative; \
float r; \
float g; \
float b; \
float a; \
bool snap; \
bool on_camera; 

node_from_field_list(StaticSprite)

// For more efficiently attaching single-sprites to nodes.
void process_Sprite(void* self, void* UNUSED_tree);
void process_StaticSprite(void *self, void *UNUSED_tree);