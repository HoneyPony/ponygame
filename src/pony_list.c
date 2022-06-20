#include "pony_list.h"

#include <stdlib.h>
#include <string.h>

void *ls_pushing_ptr(void *list_ptr, size_t item_size) {
	list_of(void) *ptr;
	ptr = list_ptr;

	if(ptr->at == NULL) {
		ptr->alloc = 8;
		ptr->length = 0;
		ptr->at = malloc(ptr->alloc * item_size);
	}
	else if(ptr->length + 1 > ptr->alloc) {
		ptr->alloc *= 2;
		ptr->at = realloc(ptr->at, item_size * ptr->alloc);
	}

	uint8_t *buf = ptr->at;
	size_t index = ptr->length++;
	return buf + (item_size * index);
}

void *ls_popping_ptr(void *list_ptr, size_t item_size) {
	list_of(void) *ptr;
	ptr = list_ptr;

	if(ptr->length == 0) {
		return NULL; // Not the best solution, honestly...
	}

	ptr->length -= 1;

	uint8_t *buf = ptr->at;
	return buf + (item_size * ptr->length);
}

void ls_delete_thru_size(void *list_ptr, size_t item_size, size_t index) {
	list_of(void) *ptr;
	ptr = list_ptr;

	if(!ptr->at) return;

	size_t items_to_move = ptr->length - index - 1;
	
	if(items_to_move > 0) {
		uint8_t *start = ptr->at;
		memmove(start + (index * item_size), start + ((index + 1) * item_size), items_to_move * item_size);
	}
	
	ptr->length -= 1;
}