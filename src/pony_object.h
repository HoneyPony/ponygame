#pragma once

#include "pony_clib.h"

/* Bits layout: 
 * Bits 0-3: prefix size (divided by 8)
 * 
 */

// Assumption: prefix MUST be a multiple of 8
// Object size may be aligned to anything, it will be rounded up
#define obj_stride_from_sizes(obj_size, prefix_size)\
(((((obj_size) + 7) / 8) * 8) + prefix_size + /*sizeof(uint64_t)*/ 8)

#define obj_stride(ObjTy, PrefixTy)\
obj_stride_from_sizes(sizeof(ObjTy), sizeof(PrefixTy))

void *obj_alloc_from_sizes(size_t obj_size, size_t prefix_size);

#define obj_alloc(ObjTy, PrefixTy)\
obj_alloc_from_sizes(sizeof(ObjTy), sizeof(PrefixTy))

#define obj_alloc_no_prefix(ObjTy)\
obj_alloc_from_sizes(sizeof(ObjTy), 0)

/**
 * @brief Reallocates the "object" portion of an object. The prefix is considered
 * to stay the same size. If you need to resize the prefix, you will have to
 * implement custom logic in order to perform the needed copying.
 * 
 * @param obj 
 * @param obj_size 
 * @return void* 
 */
void *obj_realloc(void *obj, size_t obj_size);

void *obj_prefix_ptr(void *object);

#define obj_prefix_ptr_as(object, CastTy)\
((CastTy*)obj_prefix_ptr(object))

uint64_t *obj_bits_ptr(void *object);

void *obj_self_ptr_from_size(void *prefix, size_t prefix_size);

#define obj_self_ptr(prefix, PrefixTy)\
obj_self_ptr_from_size((prefix), sizeof(PrefixTy))

size_t objbits_get_prefix_size(void *object);
void objbits_set_prefix_size(void *object, size_t size);

#define obj_assert_prefix_type(Ty)\
_Static_assert(sizeof(Ty) % 8 == 0, "Prefix type must have size divisible by 8");
