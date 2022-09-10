#pragma once

#include "pony_math.h"
#include "pony_node.h"

extern vec2 get_basis_x_fast(AnyNode *ptr);
extern vec2 get_basis_y_fast(AnyNode *ptr);

extern void node_force_compute_transform(Node *node);