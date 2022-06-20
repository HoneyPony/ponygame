#pragma once

#define mathdef1(name, arg)\
_Generic((arg),\
	int: name ## _i,\
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
