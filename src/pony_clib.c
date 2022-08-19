#include "pony_clib.h"

#include <stdlib.h>
#include <stdio.h>

void *pony_realloc(void *old, size_t new_size) {
	if(new_size == 0) {
		pony_free(old);
		return NULL;
	}

	void *result = realloc(old, new_size);
	
	if(result == NULL) {
		puts("[ponygame] allocation failure. exiting");
		exit(-1);
	}

	return result;
}

void *pony_malloc(size_t size) {
	return pony_realloc(NULL, size);
}

// Not entirely sure why I'm doing this anymore... I guess to possibly have
// some more game-tuned malloc functions...?
void *pony_calloc(size_t item_count, size_t item_size) {
	return calloc(item_count, item_size);
}

void pony_free(void *ptr) {
	free(ptr);
}