#include "pony_node.h"
#include "pony_math.h"

#define DET(x) raw_transform_determinant(x)
#define TOO_SMALL(det) (abs(det) < 0.0000001)

const RawTransform *node_get_parent_transform(Node *node) {
	if(node && node->parent) return &node->parent->internal.transform;

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
	if(parent_updated || node->internal.matrix_dirty) {
		raw_transform_compute(
			&node->internal,
			&node->internal.transform,
			node_get_parent_transform(node));
		return true;
	}
	return false;
}

vec2 get_lpos(void *ptr) {
	Node *node = ptr;

	return node->internal.translate;	
}

vec2 get_gpos(void *ptr) {
	Node *node = ptr;
	node_update_transform(node);

	return raw_transform_col2(node->internal.transform);
}

vec2 get_lscale(void *ptr) {
	Node *node = ptr;

	return node->internal.scale;
}
//vec2 get_gscale(void *node);
float get_lrot(void *ptr) {
	Node *node = ptr;
	return node->internal.rotate;
}

// Returns the arc-tangent of the first column of the global transform matrix.
// As such, may give strange results when working with skewed transforms (i.e.
// a rotated child of a scaled object.)
float get_grot(void *ptr) {
	Node *node = ptr;
	node_update_transform(node);
	RawTransform *t = &node->internal.transform;

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
	node->internal.translate = pos;
	node->internal.matrix_dirty = 1;
}

void set_gpos(void *ptr, vec2 pos) {
	Node *node = ptr;

	// Need to update the parent transform, if there is one. The parent
	// transform is going to be used to compute how our new local translation
	// will work to fulfill the proper global translation.
	//
	// Note that we don't care about our local matrix in this case, as our
	// local scale and rotation do not affect our position.
	if(node->parent) {
		node_update_transform(node->parent);
	}

	// Used for computing the new local translation. This is the most used
	// matrix, so gets the 't' label.
	const RawTransform *t = node_get_parent_transform(node);

	// Update the node global transform matrix with the new global data.
	node->internal.transform.c = pos.x;
	node->internal.transform.f = pos.y;

	// Update local translation
	// In this computation, we use our parent transform for almost everything.
	// In order to be careful about correctness, the parent transform is labeled
	// 't' and no other variables are used; this should hopefully be clear.
	float denom = DET(t);
	if(TOO_SMALL(denom)) {
		return;
	}
	// The new translation.
	// This math comes from:
	// https://www.wolframalpha.com/input?i=%5B%5Ba%2Cb%2Cc%5D%2C%5Bd%2Ce%2Cf%5D%2C%5B0%2C0%2C1%5D%5D%5E-1*%5Bx%2Cy%2C1%5D
	vec2 translate;
	translate.x = (-t->c * t->e + t->b * t->f);
	translate.x += t->e * pos.x - t->b * pos.y;
	translate.y = (t->c * t->d - t->a * t->f);
	translate.y += t->a * pos.y - t->d * pos.x;
	translate.x /= denom;
	translate.y /= denom;

	// IMPORTANT: Update this value in the node.
	node->internal.translate = translate;
}

void set_lscale(void *ptr, vec2 scale) {
	Node *node = ptr;
	node->internal.scale = scale;
	node->internal.matrix_dirty = 1;
}
//vec2 set_gscale(void *node, vec2 scale);
void set_lrot(void *ptr, float rad) {
	Node *node = ptr;
	node->internal.rotate = rad;
	node->internal.matrix_dirty = 1;
}

void set_grot(void *ptr, float rad) {
	Node *node = ptr;
	// How much the local rotation needs to be increased by to set that global
	// rotation.
	// Will give strange results if the determinant is zero, but will not
	// cause an immediate problem necessarily.
	float difference = get_grot(ptr) - rad;

	node->internal.rotate += difference;
	node->internal.matrix_dirty = 1;
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

	return raw_transform_xform(&node->internal.transform, point);
}

vec2 global_to_local(AnyNode *ptr, vec2 point) {
	Node *node = ptr;
	node_update_transform(node);

	return raw_transform_inverse_xform(&node->internal.transform, point);
}

vec2 get_basis_x(AnyNode *ptr) {
	Node *node = ptr;
	node_update_transform(node);

	return raw_transform_col0(node->internal.transform);
}

vec2 get_basis_y(AnyNode *ptr) {
	Node *node = ptr;
	node_update_transform(node);

	return raw_transform_col1(node->internal.transform);
}

void ltranslate(AnyNode *node, vec2 offset) {
	vec2 pos = get_lpos(node);
	pos.x += offset.x; // TODO: Implement vector math...
	pos.y += offset.y; 
	set_lpos(node, pos);
}

void gtranslate(AnyNode *node, vec2 offset) {
	vec2 pos = get_gpos(node);
	pos.x += offset.x;
	pos.y += offset.y;
	set_gpos(node, offset);
}