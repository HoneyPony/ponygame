#pragma once

typedef struct {
	float x;
	float y;
} vec2;

inline vec2 vxy(float x, float y) {
	return (vec2){ x, y };
}

extern float dot_v2(vec2 a, vec2 b);

#define dot(a, b)\
_Generic((a),\
	vec2: dot_v2\
)(a, b)

#define mathdef1(name, arg)\
_Generic((arg),\
	int: name ## _i,\
	float: name ## _f,\
	double: abs ## _d\
)(arg)

#define math_def_flt_1(name, arg)\
_Generic((arg),\
	float: name ## _f,\
	double: abs ## _d\
)(arg)

#define mathdef3(name, a, b, c)\
_Generic((a),\
	int: name ## _i,\
	float: name ## _f,\
	double: abs ## _d\
)(a, b, c)

extern int abs_i(int x);
extern float abs_f(float x);
extern double abs_d(double x);

#define abs(x) mathdef1(abs, x)

extern int clamp_i(int in, int low, int high);
extern float clamp_f(float in, float low, float high);
extern double clamp_d(double in, double low, double high);

#define clamp(in, low, high) mathdef3(clamp, in, low, high)

extern float atan2_f(float y, float x);
extern double atan2_d(double y, double x);

#define atan2(y, x)\
_Generic((y),\
	float: atan2_f,\
	double: atan2_d\
)(y, x)

extern int rad2deg_i(int x);
extern float rad2deg_f(float x);
extern double rad2deg_d(double x);

#define rad2deg(x) mathdef1(rad2deg, x)

extern int deg2rad_i(int x);
extern float deg2rad_f(float x);
extern double deg2rad_d(double x);

#define deg2rad(x) mathdef1(deg2rad, x)

extern float sin_f(float x);
extern double sin_d(float x);

#define sin(x) math_def_flt_1(sin, x)

extern float cos_f(float x);
extern double cos_d(float x):

#define cos(x) math_def_flt_1(cos, x)