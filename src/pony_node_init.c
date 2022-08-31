#include "pony_node.h"

#include "pony_log.h"
#include "pony_render.h"

#include <stdio.h>
#include <string.h>

node_meta_defines(Node)
node_meta_defines(PrinterNode)
node_meta_defines(Sprite)

void construct_Node(void *node) {
	Node *self = node;

	// Need to initialize the parent-child structure.
	self->parent = NULL;
	ls_init(self->children);

	// The transformation should be initialized to the identity.
	memcpy(&self->internal.transform, raw_transform_identity(), sizeof(RawTransform));

	// Initialize internal transform data to the identity as well.
	self->internal.rotate = 0;
	self->internal.translate = vxy(0.0, 0.0);
	self->internal.scale = vxy(1.0, 1.0);

	// Initialize flags
	self->internal.matrix_dirty = 0;
}

void process_PrinterNode(void *node, void *tree) {
	logf_info("message from PrinterNode");
}

void process_Sprite(void *node, void *tree) {
	Sprite *self = node;

	render_tex_on_node(self, &sprite_test_tex, vxy(0, 0), true);
}

void pony_init_builtin_nodes() {
	node_meta_initialize(Node, NULL, construct_Node, NULL, NULL)

	// Initialize root immediately
	root = new(Node);
	root->internal.immortal = 1;

	node_meta_initialize(PrinterNode, &node_header(Node), NULL, process_PrinterNode, NULL)

	node_meta_initialize(
		Sprite,
		&node_header(Node),
		NULL,
		process_Sprite,
		NULL
	)
}