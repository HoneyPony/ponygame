#pragma once

#include "pony_clib.h"

typedef char* str;

extern str str_blank();

extern str str_from(const char *source);

extern str str_fromf(const char *format, ...);

extern str str_dupe(const str other);

extern void str_clear(str string);

extern void str_free(str string);

extern uint32_t str_length(str string);

extern void str_push_by_ptr(str *string, char c);

#define str_push(str, c)\
str_push_by_ptr(&(str), c)

extern void str_append_str_by_ptr(str *string, str append);

extern void str_append_cstr_by_ptr(str *string, const char *cstr);

#define str_append_str(str, append)\
str_append_str_by_ptr(&(str), append)

#define str_append_cstr(str, append)\
str_append_cstr_by_ptr(&(str), append)

extern char str_pop(str string);

extern void str_rewind(str string, int count);

extern void str_writef_by_ptr(str *string, const char* format, ...);

#define str_writef(str, format, ...)\
str_writef_by_ptr(&(str), format, __VA_ARGS__)

extern bool cstr_has_prefix(const char *str, const char *prefix);

extern bool cstr_has_suffix(const char *str, const char *suffix);

extern bool str_eq_cstr(const str str, const char *cstr);

extern void str_trim_front(str string, size_t count);

extern int str_rfind(str s, char c);

void str_replace(str s, char old, char new);

bool is_whitespace(char c);

extern void str_wtrim_back(str string);