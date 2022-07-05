#include "pony_log.h"

#include <stdio.h>
#include <stdarg.h>

void logf_error(const char *format, ...) {
	va_list arg_list;
	va_start(arg_list, format);

	fputs("!! ", stderr);
	vfprintf(stderr, format, arg_list);
	fputs("\n", stderr);

	va_end(arg_list);
}

void logf_warn(const char *format, ...) {
	va_list arg_list;
	va_start(arg_list, format);

	fputs("!- ", stderr);
	vfprintf(stderr, format, arg_list);
	fputs("\n", stderr);

	va_end(arg_list);
}

void logf_info(const char *format, ...) {
	va_list arg_list;
	va_start(arg_list, format);

	fputs(">> ", stderr);
	vfprintf(stderr, format, arg_list);
	fputs("\n", stderr);

	va_end(arg_list);
}

void logf_verbose(const char *format, ...) {
	va_list arg_list;
	va_start(arg_list, format);

	fputs("-- ", stderr);
	vfprintf(stderr, format, arg_list);
	fputs("\n", stderr);

	va_end(arg_list);
}

/*void logf(const char *format, ...) {
	va_list arg_list;
	va_start(arg_list, format);

	fputs("   ", stderr);
	vfprintf(stderr, format, arg_list);
	fputs("\n", stderr);

	va_end(arg_list);
}*/