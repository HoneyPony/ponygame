#include "ponygame.h"
#include <stdio.h>

void print_list(list_of(int) my_list) {
	printf("--- list(%d) ---\n", ls_length(my_list));

	for(int i = 0; i < ls_length(my_list); ++i) {
		printf("[%2d] = %d\n", i, my_list[i]);
	}
}

void test() {
	ls_declare_init(int, my_list);

	for(int i = 0; i < 20; ++i) {
		ls_push(my_list, i);
	}

	print_list(my_list);

	for(int i = 0; i < 3; ++i) {
		printf("pop = %d\n", ls_pop(my_list));
	}

	print_list(my_list);

	for(int i = 0; i < 4; ++i) {
		ls_delete(my_list, 4);
	}

	print_list(my_list);
}