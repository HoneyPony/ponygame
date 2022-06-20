#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

//typedef unsigned long long size_t;

extern void *pony_realloc(void *old, size_t new_size);
extern void *pony_malloc(size_t size);
extern void  pony_free(void *ptr);

#define genvar(name)\
genvar_ ## name ## _ ## __LINE__ 

#define macro_concat2(a, b) a ## b
#define macro_concat1(a, b) macro_concat2(a, b)
#define macro_concat(a, b) macro_concat1(a, b)