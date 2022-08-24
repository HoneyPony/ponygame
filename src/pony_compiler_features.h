#pragma once

// Possibly useful for aliasing functions rather than defining them...
// Currently unused, however.
#ifdef __GNUC__
	#define alias_function(name)\
	__attribute__((alias(#name)))

	#define alias_implement(...)
#else
	#define alias_function(name)

	#define alias_implement(...) __VA_ARGS__
#endif

// Unused parameters: Very helpful for suppressing annoying warnings.
#ifdef __GNUC__
	#define UNUSED __attribute__((unused))
#else
	#define UNUSED
#endif