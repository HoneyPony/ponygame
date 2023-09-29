#include <stdio.h>
#include <string.h>

#include "pony_node.h"

#include "pony_log.h"
#include "pony_render.h"
#include "pony_compiler_features.h"

#include "node_sprite.h"

#include "ponygame.h"

node_meta_defines(Node)
node_meta_defines(PrinterNode)
node_meta_defines(Sprite)
node_meta_defines(StaticSprite)

// TODO: Determine if we want non-fixed updates of any kind.
// Note that this specifically depends on the fact that we are (trying) to have
// exactly 60 frames per second
//
// This is defined here because this is where it is used, and also because
// the 'pony' executable is currently linked to this even though it doesn't
// need to be, etc...
float get_dt() { return 1/60.0; }

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

	// Visible by default
	self->visible = true;
}

void process_PrinterNode(UNUSED void *node, UNUSED void *tree) {
	logf_info("message from PrinterNode");
}

void construct_Sprite(void *node) {
	Sprite *self = node;
	self->accumulator = 0;
	self->current_frame = 0;
	self->snap = true;

	self->r = 1;
	self->g = 1;
	self->b = 1;
	self->a = 1;
}

void process_Sprite(void *node, UNUSED void *tree) {
	Sprite *self = node;

	if(!self->current_animation) return;
	// Zero frames = nothing to display or update

	AnimHandle *anim = self->current_animation;

	if(anim->frame_count == 0) return;

	if(self->current_frame > anim->frame_count) {
		self->current_frame = 0;
	}

	AnimFrame *frame = &anim->frames[self->current_frame];

	// Store accumulated ms
	self->accumulator += get_dt() * 1000.0;
	if(self->accumulator > frame->time_ms) {
		self->accumulator -= frame->time_ms;
		self->current_frame += 1;
		self->current_frame %= anim->frame_count;
	}

	// Update frame after updating accumulator
	frame = &anim->frames[self->current_frame];

	vec2 frame_size = frame->texture.px_size;
	vec2 center = mul(frame_size, 0.5);

	// For now: only base snapping on local scale amount.
	// We could base this, at least to some degree, on the global scale amount--
	// but this should work reasonably well, just for downscaling.
	vec2 lscale = get_lscale(self);

	TexRenderer tr = {
		(Node*)self,
		self->snap_relative,
		&frame->texture,
		center,
		// Base snapping on whether the frame size is even or odd, for both
		// dimensions. This means that an unscaled, unrotated image will be
		// placed on exact pixel coordinates.
		snap_for_dimension_checked(frame_size.x * lscale.x),
		snap_for_dimension_checked(frame_size.y * lscale.y),

		self->r, self->g, self->b, self->a,

		self->on_camera
	};
	render_tex_on_node(tr);
}

void construct_StaticSprite(void *node) {
	StaticSprite *self = node;
	self->snap = true;

	
	self->r = 1;
	self->g = 1;
	self->b = 1;
	self->a = 1;
}

void process_StaticSprite(void *node, UNUSED void *tree) {
	StaticSprite *self = node;

	if(!self->texture) return;

	vec2 center = self->texture->px_size;
	center = mul(center, 0.5);

	TexHandle *frame = self->texture;
	vec2 frame_size = frame->px_size;
	vec2 lscale = get_lscale(self);

	TexRenderer tr = {
		(Node*)self,
		self->snap_relative,
		frame,
		center,
		// Base snapping on whether the frame size is even or odd, for both
		// dimensions. This means that an unscaled, unrotated image will be
		// placed on exact pixel coordinates.
		snap_for_dimension_checked(frame_size.x * lscale.x),
		snap_for_dimension_checked(frame_size.y * lscale.y),

		self->r, self->g, self->b, self->a,

		self->on_camera
	};
	render_tex_on_node(tr);
}

void pony_init_builtin_nodes() {
	ls_init(node_header_list);

	node_meta_initialize(Node, NULL, construct_Node, NULL, NULL, BLOCKS_SMALL)

	// Initialize root immediately
	root = new(Node);
	root->internal.immortal = 1;

	node_meta_initialize(PrinterNode, &node_header(Node), NULL, process_PrinterNode, NULL, BLOCKS_TINY)

	node_meta_initialize(
		Sprite,
		&node_header(Node),
		construct_Sprite,
		process_Sprite,
		NULL,
		BLOCKS_LARGE
	)

	node_meta_initialize(
		StaticSprite,
		&node_header(Node),
		construct_StaticSprite,
		process_StaticSprite,
		NULL,
		BLOCKS_LARGE
	)
}