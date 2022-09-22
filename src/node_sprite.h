#pragma once

#include "pony_node.h"
#include "pony_render.h"

#define FieldList_StaticSprite \
FieldList_Node \
TexHandle *texture; 

node_from_field_list(StaticSprite)