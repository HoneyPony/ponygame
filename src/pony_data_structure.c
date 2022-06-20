#include "pony_data_structure.h"

#include <string.h>
#include <stdio.h>

typedef struct DSPrefix DSPrefix;

void *ds_list_new_from_size(size_t item_size) {
	size_t alloc = 2;
	
	void *ptr = obj_alloc_from_sizes(item_size * alloc, sizeof(DSPrefix));
	
	DSPrefix *prefix = obj_prefix_ptr(ptr);
	prefix->alloc = alloc;
	prefix->length = 0;

	/*pony_bits_set_type(*bits, PONY_TY_LIST);
	pre->alloc = alloc;
	pre->length = 0;*/
	
	return ptr;
}

static void* ds_list_expand(DSPrefix *prefix, void *ds, size_t item_size) {
	size_t alloc = prefix->alloc * 2;

	ds = obj_realloc(ds, alloc * item_size);
	prefix = obj_prefix_ptr(ds);

	prefix->alloc = alloc;
	
	return ds;
}

#define BASE(ptr) ((uint8_t*)ptr)

void *ds_push_thru_ptr(void *ds, size_t item_size, void *item) {
#ifdef PONY_DEBUG
	if(!ds) {
		puts("warning: appending to null list. did you forget to initialize it?");
		return NULL;
	}
#endif
	
	DSPrefix *t = obj_prefix_ptr(ds);
	
	if(t->length == t->alloc) {
		ds = ds_list_expand(t, ds, item_size);
		t = obj_prefix_ptr(ds);
	}

	uint8_t *buf = ds;
	
	memcpy(buf + item_size * t->length, item, item_size);
	
	t->length += 1;
	
	return ds;
}

void intl_pop(void *ds, size_t item_size, void *out) {
	DSPrefix *t = obj_prefix_ptr(ds);
	
	if(t->length < 1) return;
	
	t->length -= 1;
	memcpy(out, BASE(ds) + item_size * t->length, item_size);
}

int intl_has(void *ds, size_t item_size, void *cmp) {
	DSPrefix *t = obj_prefix_ptr(ds);
	
	for(size_t i = 0; i < t->length; ++i) {
		int check = memcmp(BASE(ds) + item_size * i, cmp, item_size);
		if(!check) return 1;
	}
	return 0;
}

void intl_delete(void *ds, size_t item_size, size_t index) {
	DSPrefix *t = obj_prefix_ptr(ds);
	if(index >= t->length) return;
	
	size_t items_to_move = t->length - index - 1;
	
	if(items_to_move > 0) {
		memmove(BASE(ds) + (index * item_size), BASE(ds) + ((index + 1) * item_size), items_to_move * item_size);
	}
	
	t->length -= 1;
}

uint32_t ds_length(void *ds) {
	return obj_prefix_ptr_as(ds, DSPrefix)->length;
}