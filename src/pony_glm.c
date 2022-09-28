#include "pony_glm.h"

#include <string.h>

#define at(x, y) ((x * 4) + y)

void mat4_zero(mat4 *out) {
	memset(out->data, 0, sizeof(float) * 16);
}

void mat4_ortho(mat4 *out,
	float left, float right,
	float bottom, float top,
	float near, float far)
{
	float rml = right - left;
	float tmb = top - bottom;
	float fmn = far - near;

	float rpl = right + left;
	float tpb = top + bottom;
	float fpn = far + near;

	mat4_zero(out);
	out->data[at(0, 0)] = 2.0 / rml;
	out->data[at(1, 1)] = 2.0 / tmb;
	out->data[at(2, 2)] = -2.0 / fmn;

	out->data[at(3, 0)] = -rpl / rml;
	out->data[at(3, 1)] = -tpb / tmb;
	out->data[at(3, 2)] = -fpn / fmn;
	out->data[at(3, 3)] = 1.0;
}

void mat4_translation(mat4 *out, float x, float y, float z) {
	mat4_zero(out);

	out->data[at(0, 0)] = 1.0;
	out->data[at(1, 1)] = 1.0;
	out->data[at(2, 2)] = 1.0;
	out->data[at(3, 3)] = 1.0;

	out->data[at(3, 0)] = x;
	out->data[at(3, 1)] = y;
	out->data[at(3, 2)] = z;
}

void mat4_multiply(mat4 *out, const mat4 *in) {
	for(int i = 0; i < 4; ++i) {
		for(int j = 0; j < 4; ++j) {
			float next = 0.0;
			for(int index = 0; index < 4; ++index) {
				next += out->data[at(index, j)] * in->data[at(i, index)];
			}
			out->data[at(i, j)] = next;
		}
	}
}