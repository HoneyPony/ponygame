#pragma once

typedef struct {
	// Column-major ordering
	float data[16];
} mat4;

extern void mat4_ortho(mat4 *out,
	float left, float right,
	float bottom, float top,
	float near, float far);

extern void mat4_zero(mat4 *out);

extern void mat4_translation(mat4 *out, float x, float y, float z);

extern void mat4_multiply(mat4 *out, const mat4 *by);