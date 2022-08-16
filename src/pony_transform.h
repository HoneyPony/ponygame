#pragma once

#include "pony_math.h"

typedef struct {
	vec2 translate;
	vec2 scale;
	float rotate;

	union {
		// Layout: column major. Lets us extract columns as vec2s.
		struct {
			float a;
			float d;

			float b;
			float e;

			float c;
			float f;
		};

		struct {
			vec2 col0;
			vec2 col1;
			vec2 col2;
		};

		float matrix[6];
	};

	//int8_t translate_dirty;
	//int8_t scale_dirty;
	//int8_t rotate_dirty;
	int8_t matrix_dirty;
} RawTransform;

void raw_transform_copy(RawTransform *target, const RawTransform *from);

extern const RawTransform *raw_transform_identity();

void raw_transform_compute(RawTransform *target, const RawTransform *parent);

float raw_transform_determinant(RawTransform *tform);

extern vec2 raw_transform_xform(const RawTransform *tform, vec2 input);
extern vec2 raw_transform_inverse_xform(const RawTransform *tform, vec2 input);