#include "pony_list.h"

#include "pony_prefix.h"

#include <string.h>

typedef struct {
	int32_t length;
	int32_t alloc;
} ListPrefix;

void *ls_init_internal(size_t item_size) {
	size_t alloc = 4;
	
	void *ptr = pony_malloc(prefix_alloc_size(alloc * item_size, ListPrefix));
	
	ListPrefix *prefix = ptr;
	prefix->alloc = alloc;
	prefix->length = 0;
	
	return prefix_obj_ptr(ptr, ListPrefix);
}

static ListPrefix* ls_expand(ListPrefix *prefix, size_t item_size) {
	size_t alloc = prefix->alloc * 2;

	size_t size = prefix_alloc_size(alloc * item_size, ListPrefix);

	prefix = pony_realloc(prefix, size);

	prefix->alloc = alloc;
	
	return prefix;
}

#define BASE(ptr) ((uint8_t*)ptr)

void *ls_push_internal(void *list, size_t item_size, void *item) {
#ifdef PONY_DEBUG
	if(!list) {
		puts("warning: appending to null list. did you forget to initialize it?");
		return NULL;
	}
#endif
	
	ListPrefix *prefix = prefix_ptr(list, ListPrefix);
	
	if(prefix->length == prefix->alloc) {
		prefix = ls_expand(prefix, item_size);
		list = prefix_obj_ptr(prefix, ListPrefix);
	}

	uint8_t *buf = list;
	
	memcpy(buf + item_size * prefix->length, item, item_size);
	
	prefix->length += 1;
	
	return list;
}

void ls_pop_internal(void *list, size_t item_size, void *out) {
	ListPrefix *prefix = prefix_ptr(list, ListPrefix);
	
	if(prefix->length < 1) return;
	
	prefix->length -= 1;
	memcpy(out, BASE(list) + item_size * prefix->length, item_size);
}

bool ls_has_internal(void *list, size_t item_size, void *cmp) {
	ListPrefix *prefix = prefix_ptr(list, ListPrefix);
	
	for(size_t i = 0; i < prefix->length; ++i) {
		int check = memcmp(BASE(list) + item_size * i, cmp, item_size);
		if(!check) return 1;
	}
	return 0;
}

void ls_delete_internal(void *list, size_t item_size, size_t index) {
	ListPrefix *prefix = prefix_ptr(list, ListPrefix);
	if(index >= prefix->length) return;
	
	size_t items_to_move = prefix->length - index - 1;
	
	if(items_to_move > 0) {
		memmove(BASE(list) + (index * item_size), BASE(list) + ((index + 1) * item_size), items_to_move * item_size);
	}
	
	prefix->length -= 1;
}

uint32_t ls_length(void *list) {
	ListPrefix *prefix = prefix_ptr(list, ListPrefix);
	return prefix->length;
}