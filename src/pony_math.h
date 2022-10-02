#pragma once

// NOTE: For code to compile on the web, this file MUST be included AFTER
// stdlib.h, and maybe other standard headers.
//
// In particular, the 'abs' macro that this file defines will cause all sorts
// of chaos when it overrides parts of the standard library.

#include <tgmath.h>

typedef struct {
	float x;
	float y;
} vec2;

static inline vec2 vxy(float x, float y) {
	return (vec2){ x, y };
}

static inline float dot_v2(vec2 a, vec2 b) {
	return a.x * b.x + a.y * b.y;
}

static inline vec2 round_v2(vec2 v) {
	return vxy(roundf(v.x), roundf(v.y));
}

static inline vec2 floor_v2(vec2 v) {
	return vxy(floor(v.x), floor(v.y));
}


static inline vec2 mul(vec2 a, float c) {
	return vxy(a.x * c, a.y * c);
}

static inline vec2 add(vec2 a, vec2 b) {
	return vxy(a.x + b.x, a.y + b.y);
}

static inline vec2 sub(vec2 a, vec2 b) {
	return vxy(a.x - b.x, a.y - b.y);
}


static inline float length(vec2 v) {
	return sqrt(v.x * v.x + v.y * v.y);
}

static inline vec2 norm(vec2 v) {
	float l = length(v);
	if(l == 0) return v;
	return vxy(v.x / l, v.y / l);
}

#define dot(a, b)\
_Generic((a),\
	vec2: dot_v2\
)(a, b)

#define mathdef1(name, arg)\
_Generic((arg),\
	int: name ## _i,\
	float: name ## _f,\
	double: name ## _d\
)(arg)

#define math_def_flt_1(name, arg)\
_Generic((arg),\
	float: name ## _f,\
	double: name ## _d\
)(arg)

#define mathdef3(name, a, b, c)\
_Generic((a),\
	int: name ## _i,\
	float: name ## _f,\
	double: name ## _d\
)(a, b, c)

#define abs(a) fabs(a)

extern int clamp_i(int in, int low, int high);
extern float clamp_f(float in, float low, float high);
extern double clamp_d(double in, double low, double high);

#define clamp(in, low, high) mathdef3(clamp, in, low, high)

extern int rad2deg_i(int x);
extern float rad2deg_f(float x);
extern double rad2deg_d(double x);

#define rad2deg(x) mathdef1(rad2deg, x)

extern int deg2rad_i(int x);
extern float deg2rad_f(float x);
extern double deg2rad_d(double x);

#define deg2rad(x) mathdef1(deg2rad, x)

extern int lerp_i(int a, int b, int t);
extern float lerp_f(float a, float b, float t);
extern double lerp_d(double a, double b, double t);

#define lerp(a, b, t) mathdef3(lerp, a, b, t)

// Need to redefine round
#undef round
#define round(x)\
_Generic((x),\
	long: lround,\
	float: roundf,\
	double: round,\
	vec2: round_v2\
)(x)

#define PI 3.14159265359 