#pragma once

#include "pony_math.h"
#include "pony_clib.h"

typedef struct {
	float a;
	float d;

	float b;
	float e;

	float c;
	float f;
} RawTransform;

extern const RawTransform *raw_transform_identity();

// Forward declare
struct NodeInternal;
void raw_transform_compute(const struct NodeInternal *node_data, RawTransform *target, const RawTransform *parent);

float raw_transform_determinant(const RawTransform *tform);

extern vec2 raw_transform_xform(const RawTransform *tform, vec2 input);
extern vec2 raw_transform_inverse_xform(const RawTransform *tform, vec2 input);

#define raw_transform_col0(t)\
vxy((t).a, (t).d)

#define raw_transform_col1(t)\
vxy((t).b, (t).e)

#define raw_transform_col2(t)\
vxy((t).c, (t).f)
