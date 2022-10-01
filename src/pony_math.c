#include "pony_math.h"

#undef abs
#undef atan2
#undef cos
#undef sin

#define RAD_2_DEG 57.29577951308232087679815481410517033240547246656432154916024386120285
#define DEG_2_RAD 0.01745329251994329576923690768488612713442871888541725456097191440171009

#include <math.h>

/* abs */

/*int abs_i(int x) {
	return abs(x);
}

float abs_f(float x) {
	return fabsf(x);
}

double abs_d(double d) {
	return fabs(d);
}
*/

/* atan2 */

/*
float atan2_f(float y, float x) {
	return atan2f(y, x);
}

double atan2_d(double y, double x) {
	return atan2(y, x);
}
*/

/* clamp */

int clamp_i(int in, int low, int high) {
	if(in < low) return low;
	if(in > high) return high;
	return in;
}

float clamp_f(float in, float low, float high) {
	if(in < low) return low;
	if(in > high) return high;
	return in;
}

double clamp_d(double in, double low, double high) {
	if(in < low) return low;
	if(in > high) return high;
	return in;
}

/* sin */

/*
float sin_f(float x) {
	return sinf(x);
}

double sin_d(double x) {
	return sin(x);
}
*/

/* cos */

/*
float cos_f(float x) {
	return cosf(x);
}

double cos_d(double x) {
	return cos(x);
}
*/

/* rad2deg */

int rad2deg_i(int x) {
	return (int)(RAD_2_DEG * x);
}

float rad2deg_f(float x) {
	return RAD_2_DEG * x;
}

double rad2deg_d(double x) {
	return RAD_2_DEG * x;
}

/* deg2rad */

int deg2rad_i(int x) {
	return (int)(DEG_2_RAD * x);
}

float deg2rad_f(float x) {
	return DEG_2_RAD * x;
}

double deg2rad_d(double x) {
	return DEG_2_RAD * x;
}

/* lerp */

// TODO: integer lerp doesn't make any sense
int lerp_i(int a, int b, int t) {
	return a * (1 - t) + b * t;
}

float lerp_f(float a, float b, float t) {
	return a * (1 - t) + b * t;
}

double lerp_d(double a, double b, double t) {
	return a * (1 - t) + b * t;
}