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

void pony_free(void *ptr) {
	free(ptr);
}