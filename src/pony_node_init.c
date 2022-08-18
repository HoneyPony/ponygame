#include "pony_node.h"

#include <stdio.h>

node_meta_defines(Node)

void construct_Node(void *node) {
	Node *self = node;

	// Need to initialize the parent-child structure.
	self->parent = NULL;
	ls_init(self->children);

	// The transformation should be initialized to the identity.
	raw_transform_copy(&self->raw_tform, raw_transform_identity());
}

void pony_init_builtin_nodes() {
	node_meta_initialize(Node, NULL, construct_Node, NULL)
}