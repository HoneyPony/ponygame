#include "pony_node.h"

#include <stdio.h>

node_meta_defines(Node)

void construct_Node(void *node) {
	Node *self = node;

	self->parent = NULL;
	ds_init_list(self->children);
}

void pony_init_builtin_nodes() {
	node_meta_initialize(Node, NULL, construct_Node)
}