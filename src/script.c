#include "ponygame.h"
#include <stdio.h>

void print_list(list_of(int) *my_list) {
	printf("--- list(%d) ---\n", my_list->length);

	for(int i = 0; i < my_list->length; ++i) {
		printf("[%2d] = %d\n", i, my_list->at[i]);
	}
}

void test() {
	list_of(int) my_list = { 0 };

	for(int i = 0; i < 20; ++i) {
		ls_push(my_list, i);
	}

	print_list(&my_list);

	for(int i = 0; i < 3; ++i) {
		printf("pop = %d\n", ls_pop(my_list));
	}

	print_list(&my_list);

	for(int i = 4; i < 8; ++i) {
		ls_delete(my_list, i);
	}

	print_list(&my_list);
}