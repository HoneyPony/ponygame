#include "pony_string.h"

#include "pony_prefix.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

typedef struct {
	uint32_t length;
	uint32_t alloc;
} StrPrefix;

void str_free(str string) {
	pony_free(prefix_ptr(string, StrPrefix));
}

static StrPrefix *alloc_prefix(size_t alloc, size_t length) {
	size_t total = prefix_alloc_size(alloc * sizeof(char), StrPrefix);
	
	StrPrefix *prefix = pony_malloc(total);
	prefix->length = length;
	prefix->alloc = alloc;

	return prefix;
}

str str_blank() {
	size_t alloc = 8;
	
	StrPrefix *prefix = alloc_prefix(alloc, 0);

	str result = prefix_obj_ptr(prefix, StrPrefix);
	result[0] = '\0';

	return result;
}

str str_from(const char *source) {
	size_t needed = strlen(source) + 1;

	StrPrefix *prefix = alloc_prefix(needed, needed - 1);
	
	str result = prefix_obj_ptr(prefix, StrPrefix);
	memcpy(result, source, needed);

	return result;
}

uint32_t str_length(str string) {
	StrPrefix *prefix = prefix_ptr(string, StrPrefix);
	return prefix->length;
}

void str_clear(str string) {
	StrPrefix *prefix = prefix_ptr(string, StrPrefix);
	prefix->length = 0;
	string[0] = '\0';
}

static StrPrefix *str_ensure(str *string, size_t needed_alloc) {
	StrPrefix *prefix = prefix_ptr(*string, StrPrefix);

	if(needed_alloc <= prefix->alloc) return prefix;

	size_t actual_alloc = 1;
	// Allocate in powers of 2
	while(actual_alloc < needed_alloc) actual_alloc <<= 1;

	prefix = pony_realloc(prefix, prefix_alloc_size(actual_alloc, StrPrefix));
	*string = prefix_obj_ptr(prefix, StrPrefix);

	return prefix;
}

void str_push_by_ptr(str *string, char c) {
	StrPrefix *prefix = prefix_ptr(*string, StrPrefix);

	// We will need at least length + 2 bytes overall (due to the terminator)
	size_t needed = prefix->length + sizeof(c) + sizeof('\0');

	prefix = str_ensure(string, needed);

	(*string)[prefix->length++] = c;
	(*string)[prefix->length] = '\0';
}


void str_append_str_by_ptr(str *string, str append) {
	StrPrefix *prefix = prefix_ptr(*string, StrPrefix);

	size_t a_len = str_length(append);

	size_t needed = prefix->length + a_len + sizeof('\0');

	prefix = str_ensure(string, needed);

	// Also copy the null terminator
	memcpy((*string) + prefix->length, append, a_len + 1);

	prefix->length += a_len;
}

void str_append_cstr_by_ptr(str *string, const char* append) {
	StrPrefix *prefix = prefix_ptr(*string, StrPrefix);

	size_t a_len = strlen(append);

	size_t needed = prefix->length + a_len + sizeof('\0');

	prefix = str_ensure(string, needed);

	// Also copy the null terminator
	memcpy((*string) + prefix->length, append, a_len + 1);

	prefix->length += a_len;
}


char *str_last(str string) {
	return &string[str_length(string) - 1];
}

char str_pop(str string) {
	StrPrefix *prefix = prefix_ptr(string, StrPrefix);

	char result = *str_last(string);
	string[prefix->length] = '\0';

	prefix->length -= 1;

	return result;
}

void str_writef_by_ptr(str *string, const char* format, ...) {
	StrPrefix *prefix = prefix_ptr(*string, StrPrefix);

	va_list arg_list, count_list;
	va_start(arg_list, format);
	va_start(count_list, format);

	// Append_length = the length of the new string that will be appended.
	// This does not include the null terminator.
	size_t append_length = vsnprintf(NULL, 0, format, count_list);

	// Needed = the total allocation needed for the whole string. This includes
	// all the length of both strings, plus 1 null terminator.
	size_t needed = prefix->length + append_length + sizeof('\0');

	// Allowed to write = the number of characters that vsnprintf is allowed to
	// write. It needs to be able to write all the characters of the new append,
	// but it also needs to be able to write the null terminator.
	size_t allowed_to_write = append_length + 1; // Space for null terminator

	prefix = str_ensure(string, needed);

	vsnprintf((*string) + prefix->length, allowed_to_write, format, arg_list);

	va_end(arg_list);
	va_end(count_list);

	prefix->length += append_length;
	(*string)[prefix->length] = '\0';
}

str str_fromf(const char *format, ...) {
	va_list arg_list, count_list;
	va_start(arg_list, format);
	va_start(count_list, format);

	// length of string without null terminator
	size_t string_length = vsnprintf(NULL, 0, format, count_list);

	// needed = total space needed, also will be the n for printf
	size_t needed = string_length + sizeof('\0');

	StrPrefix *prefix = alloc_prefix(needed, string_length);

	str string = prefix_obj_ptr(prefix, StrPrefix);

	vsnprintf(string, needed, format, arg_list);

	va_end(arg_list);
	va_end(count_list);

	return string;
}

bool cstr_has_prefix(const char *str, const char *prefix) {
	while(*str && *prefix) {
		if(*str != *prefix) return false;

		++str;
		++prefix;

		if(*prefix == '\0') return true;
		if(*str == '\0') return false;
	}
	return false;
}