#include <stdlib.h>
#include <math.h>

#include "ponygame.h"

float randf() {
	return rand() / (float)RAND_MAX;
}

int rand_range(int i, int j) {
	return lround(randf_range(i, j));
}

float randf_range(float i, float j) {
	float t = randf();
	return i * t + j * (1 - t);
}