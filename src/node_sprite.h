#pragma once

#include "pony_node.h"
#include "pony_render.h"

#define FieldList_Sprite \
FieldList_Node \
AnimHandle *current_animation; \
int current_frame; \
float accumulator; \
bool snap; \
Node *snap_relative; \
float r; \
float g; \
float b; \
float a;

node_from_field_list(Sprite)

void sprite_play(Sprite *sprite, AnimHandle *anim);

#define FieldList_StaticSprite \
FieldList_Node \
TexHandle *texture; \
bool snap;

node_from_field_list(StaticSprite)