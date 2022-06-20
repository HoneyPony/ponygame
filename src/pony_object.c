#include "pony_object.h"

void *obj_alloc_from_sizes(size_t obj_size, size_t prefix_size) {
	size_t total_alloc = (obj_size + prefix_size + 8);

	uint8_t *block = pony_malloc(total_alloc);

	void *result = obj_self_ptr_from_size(block, prefix_size);
	uint64_t *bits = obj_bits_ptr(result);

	*bits = 0;

	objbits_set_prefix_size(result, prefix_size);

	return result;
}

void *obj_realloc(void *obj, size_t obj_size) {
	void *old_block = obj_prefix_ptr(obj);

	size_t prefix_size = objbits_get_prefix_size(obj);

	size_t total_alloc = (obj_size + prefix_size + 8);
	void *block = pony_realloc(old_block, total_alloc);

	return obj_self_ptr_from_size(block, prefix_size);
}

void *obj_self_ptr_from_size(void *object, size_t prefix_size) {
	uint8_t *ptr = object;
	ptr += (prefix_size + 8);
	return ptr;
}

void *obj_prefix_ptr(void *object) {
	uint8_t *ptr = object;
	size_t prefix_size = objbits_get_prefix_size(object);
	ptr -= (prefix_size + 8);
	return ptr;	
}

uint64_t *obj_bits_ptr(void *object) {
	uint8_t *ptr = object;
	ptr -= 8;
	return (uint64_t*)ptr;	
}

void objbits_set_prefix_size(void *object, size_t size) {
	size >>= 3; // Divide by 8

	uint64_t *bits = obj_bits_ptr(object);
	*bits = (*bits & 0xF) | (size & 0xF);
}

size_t objbits_get_prefix_size(void *object) {
	uint64_t *bits = obj_bits_ptr(object);
	return (*bits & 0xF) << 3;
}