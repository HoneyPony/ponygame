#include "pony_transform.h"

#include "pony_clib.h"
#include "pony_math.h"
#include <string.h>

void raw_transform_copy(RawTransform *target, const RawTransform *from) {
	memcpy(target, from, sizeof(RawTransform));
	target->matrix_dirty = 1;
}

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

		identity.rotate = 0;
		identity.scale = vxy(1, 1);
		identity.translate = vxy(0, 0);

		identity.matrix_dirty = 0;
	}

	return &identity;
}

static void multiply(RawTransform *target, const RawTransform *parent) {
	vec2 row1 = vxy(parent->a, parent->b);
	vec2 row2 = vxy(parent->c, parent->d);

	RawTransform result;
	result.a = dot(row1, target->col0);
	result.b = dot(row1, target->col1);
	result.c = dot(row1, target->col2) + parent->c;

	result.d = dot(row2, target->col0);
	result.e = dot(row2, target->col1);
	result.f = dot(row2, target->col2) + parent->f;

	memcpy(target->matrix, result.matrix, sizeof(result.matrix));
}

void raw_transform_compute(RawTransform *target, const RawTransform *parent) {
	float c = cos(target->rotate);
	float s = sin(target->rotate);

	target->a = target->scale.x * c;
	target->b = target->scale.y * -s;
	target->d = target->scale.x * s;
	target->e = target->scale.y * c;
	target->col2 = target->translate;
	
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