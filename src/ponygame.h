#pragma once

#include "pony_clib.h"
#include "pony_node.h"
#include "pony_string.h"
#include "pony_list.h"
#include "pony_render.h"
#include "pony_sound.h"
#include "pony_log.h"
#include "pony_fs.h"
#include "pony_input.h"

// Builtin Nodes
#include "node_sprite.h"

float get_dt();

extern int rand_range(int i, int j);
extern float randf_range(float i, float j);

//#define tree_instance(tree)
//#define instance(tree) tree_instance(tree)
