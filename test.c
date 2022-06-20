#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define list_of(Ty) struct { int32_t alloc; int32_t length; Ty *at; }
#define list_init { 0 }

void *push_ptr(void *list, size_t item_size) {
	struct {int32_t alloc; int32_t length; void *at;} *ptr;
	ptr = list;

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

#define push(list, item)\
((*(typeof((list).at))(push_ptr(&(list), sizeof(*(list).at)))) = (item))

int main() {
	list_of(int) my_list = list_init;

	for(int i = 0; i < 10; ++i) {
		push(my_list, i * 2);
	}

	for(int i = 0; i < my_list.length; ++i) {
		printf("item @ %d = %d\n", i, my_list.at[i]);
	}
}