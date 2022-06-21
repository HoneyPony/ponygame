#pragma once

/* Prefixes are usually used for data structures, such as lists, to provide a
 * convenient interface. In particular, generic data types can be relatively
 * nicely implemented in C through the following patten:
 * 
 * Consider a generic type that has a single type parameter, such as a list of
 * some type. Call this type Ty. Then, we can declare a list variable to be of
 * type Ty*. This gives us two very useful features:
 * 
 * 1. This variable may point to a totally arbitrary block of memory, because
 *    it is a pointer.
 * 2. We can use typeof() and sizeof() to get the type from a given variable,
 *    e.g. within a macro, because the variable is a pointer to that type.
 * 
 * For lists and strings, this provides the additional advantage of allowing us
 * to directly index the pointer using normal syntax.
 * 
 * To implement lists and strings, however, we need a way to store additional
 * data somewhere, storing the length of the data. This is where the prefix
 * system comes in. It is a small set of helper methods for storing the prefixed
 * data behind the rest of the data.
 */

#define prefix_alloc_size(obj_size, PrefixTy)\
((obj_size) + sizeof(PrefixTy))

#define prefix_obj_ptr(prefix_ptr, PrefixTy)\
((void*)(((uint8_t*)(prefix_ptr)) + sizeof(PrefixTy)))

#define prefix_ptr(obj_ptr, PrefixTy)\
((void*)(((uint8_t*)(obj_ptr)) - sizeof(PrefixTy)))
