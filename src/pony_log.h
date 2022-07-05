#pragma once

extern void logf_error(const char *format, ...);
extern void logf_warn(const char *format, ...);
extern void logf_info(const char *format, ...);
extern void logf_verbose(const char *format, ...);

// TODO: Determine good name for this basic logging function intended for use
// in user code.
// 'logf' is already used for a math function.
//extern void logf(const char *format, ...);