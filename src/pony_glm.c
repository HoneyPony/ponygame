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