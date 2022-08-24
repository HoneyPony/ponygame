#pragma once

#include <stdbool.h>
#include <stdio.h>

extern int tests_total;
extern int tests_in_set;

extern int tests_passed_total;
extern int tests_passed_in_set;

extern int tests_failed_total;
extern int tests_failed_in_set;

extern void test_set_begin();
extern void test_set_end(const char *name);

#define test_assert(bool_val, err_fmt, ...)\
do {\
	if(!(bool_val)) {\
		printf("Assertion failure in '%s' at line %d:\n\t" err_fmt "\n",\
			__FUNCTION__,\
			__LINE__\
			__VA_OPT__(,) __VA_ARGS__);\
		if(!test_has_failed) {\
			test_has_failed = true;\
			tests_failed_total += 1;\
			tests_failed_in_set += 1;\
		}\
	}\
} while(0)

#define test_assert_eq(left, right, fmt)\
test_assert((left) == (right), fmt, left, right)

#define test_define(name, ...)\
void name() {\
	bool test_has_failed = false;\
	tests_total += 1;\
	tests_in_set += 1;\
	{ __VA_ARGS__ }\
	if(!test_has_failed) {\
		tests_passed_total += 1;\
		tests_passed_in_set += 1;\
	}\
}

#define test_set_define(name, ...)\
void name() {\
	test_set_begin();\
	{ __VA_ARGS__ }\
	test_set_end(#name);\
}
