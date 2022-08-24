#include "pony_raw_transform.h"

#include "pony_clib.h"
#include "pony_math.h"
#include "pony_node.h"
#include <string.h>

const RawTransform *raw_transform_identity() {
	static RawTransform identity;
	static bool init = false;

	if(!init) {
		init = true;
		identity.a = 1;
		identity.b = 0;
		identity.c = 0;

		identity.d = 0;
		identity.e = 1;
		identity.f = 0;
	}

	return &identity;
}

static void multiply(RawTransform *target, const RawTransform *parent) {
	vec2 row1 = vxy(parent->a, parent->b);
	vec2 row2 = vxy(parent->d, parent->e);

	vec2 col0 = raw_transform_col0(*target);
	vec2 col1 = raw_transform_col1(*target);
	vec2 col2 = raw_transform_col2(*target);

	RawTransform result;
	result.a = dot(row1, col0);
	result.b = dot(row1, col1);
	result.c = dot(row1, col2) + parent->c;

	result.d = dot(row2, col0);
	result.e = dot(row2, col1);
	result.f = dot(row2, col2) + parent->f;

	memcpy(target, &result, sizeof(result));
}

void raw_transform_compute(const struct NodeInternal *node_data, RawTransform *target, const RawTransform *parent) {
	float c = cos(node_data->rotate);
	float s = sin(node_data->rotate);

	target->a = node_data->scale.x * c;
	target->b = node_data->scale.y * -s;
	target->d = node_data->scale.x * s;
	target->e = node_data->scale.y * c;
	
	target->c = node_data->translate.x;
	target->f = node_data->translate.y;
	
	multiply(target, parent);
}

float raw_transform_determinant(const RawTransform *tform) {
	return tform->a * tform->e - tform->b * tform->d;
}

vec2 raw_transform_xform(const RawTransform *tform, vec2 in) {
	vec2 out;
	out.x = tform->a * in.x + tform->b * in.y + tform->c;
	out.y = tform->d * in.x + tform->e * in.y + tform->f;
	return out;
}

vec2 raw_transform_inverse_xform(const RawTransform *t, vec2 in) {
	float det = raw_transform_determinant(t);

	if(abs(det) < 0.0000001) {
		return vxy(0, 0);
	}

	float x = t->b * t->f - t->c * t->e;
	x += t->e * in.x - t->b * in.y;

	float y = t->c * t->d - t->a * t->f;
	y += -t->d * in.x + t->a * in.y;

	return vxy(x, y); 
}