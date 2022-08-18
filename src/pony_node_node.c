#include "pony_node.h"
#include "pony_math.h"

#define DET(x) raw_transform_determinant(x)
#define TOO_SMALL(det) (abs(det) < 0.0000001)

const RawTransform *node_get_parent_transform(Node *node) {
	if(node && node->parent) return &node->parent->raw_tform;

	return raw_transform_identity();
}

bool node_update_transform(Node *node) {
	if(!node) return false;

	// TODO: Is there a more efficient way to ensure everything is up to date?
	// Obviously we could do the opposite... signal child nodes when their
	// parent transform changes...
	//
	// But that isn't great either.
	//
	// One particular case we could optimize for is when we are in the tick
	// function for a single node, and it is updating its own transform.
	// Perhaps what we could do is track a single "last updated node," and if
	// that node is updated again, we don't update its parents a second time.
	//
	// Then we simply invalidate that node... there is extra space for another
	// flag in the transform, so that might work.
	bool parent_updated = node_update_transform(node->parent);
	if(parent_updated || node->raw_tform.matrix_dirty) {
		raw_transform_compute(&node->raw_tform, node_get_parent_transform(node));
		return true;
	}
	return false;
}

vec2 get_lpos(void *ptr) {
	Node *node = ptr;

	return node->raw_tform.translate;	
}

vec2 get_gpos(void *ptr) {
	Node *node = ptr;
	node_update_transform(node);

	return node->raw_tform.col2;
}

vec2 get_lscale(void *ptr) {
	Node *node = ptr;

	return node->raw_tform.scale;
}
//vec2 get_gscale(void *node);
float get_lrot(void *ptr) {
	Node *node = ptr;
	return node->raw_tform.rotate;
}

// Returns the arc-tangent of the first column of the global transform matrix.
// As such, may give strange results when working with skewed transforms (i.e.
// a rotated child of a scaled object.)
float get_grot(void *ptr) {
	Node *node = ptr;
	node_update_transform(node);
	RawTransform *t = &node->raw_tform;

	float det = DET(t);
	if(TOO_SMALL(det)) {
		return 0;
	}

	return atan2(t->d, t->a);
}

float get_lrot_deg(void *ptr) {
	return rad2deg(get_lrot(ptr));
}

float get_grot_deg(void *ptr) {
	return rad2deg(get_grot(ptr));
}

void set_lpos(void *ptr, vec2 pos) {
	Node *node = ptr;
	RawTransform *t = &node->raw_tform;

	t->translate = pos;
	t->matrix_dirty = 1;
}

void set_gpos(void *ptr, vec2 pos) {
	Node *node = ptr;
	RawTransform *t = &node->raw_tform;

	t->c = pos.x;
	t->f = pos.y;

	// Update local translation
	float denom = DET(t);
	if(TOO_SMALL(denom)) {
		return;
	}
	t->translate.x = -t->c * t->e + t->b * t->f;
	t->translate.y = t->c * t->d - t->a * t->f;
	t->translate.x /= denom;
	t->translate.y /= denom;
}

void set_lscale(void *ptr, vec2 scale) {
	Node *node = ptr;
	RawTransform *t = &node->raw_tform;

	t->scale = scale;
	t->matrix_dirty = 1;
}
//vec2 set_gscale(void *node, vec2 scale);
void set_lrot(void *ptr, float rad) {
	Node *node = ptr;
	RawTransform *t = &node->raw_tform;

	t->rotate = rad;
	t->matrix_dirty = 1;
}

void set_grot(void *ptr, float rad) {
	Node *node = ptr;
	RawTransform *t = &node->raw_tform;

	// How much the local rotation needs to be increased by to set that global
	// rotation.
	// Will give strange results if the determinant is zero, but will not
	// cause an immediate problem necessarily.
	float difference = get_grot(ptr) - rad;

	t->rotate += difference;
	t->matrix_dirty = 1;
}

void set_lrot_deg(void *ptr, float deg) {
	set_lrot(ptr, deg2rad(deg));
}

void set_grot_deg(void *ptr, float deg) {
	set_grot(ptr, deg2rad(deg));
}

vec2 local_to_global(AnyNode *ptr, vec2 point) {
	Node *node = ptr;
	node_update_transform(node);

	return raw_transform_xform(&node->raw_tform, point);
}

vec2 global_to_local(AnyNode *ptr, vec2 point) {
	Node *node = ptr;
	node_update_transform(node);

	return raw_transform_inverse_xform(&node->raw_tform, point);
}

vec2 get_basis_x(AnyNode *ptr) {
	Node *node = ptr;
	node_update_transform(node);

	return node->raw_tform.col0;
}

vec2 get_basis_y(AnyNode *ptr) {
	Node *node = ptr;
	node_update_transform(node);

	return node->raw_tform.col1;
}