#include "ponygame.h"
#include <stdio.h>
/*
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
*/

/*
void test() {
	str my_str = str_fromf("%s%s", "ABC", " : ");

	for(char c = 'a'; c <= 'z'; ++c) {
		str_push(my_str, c);

		printf("%2d [%s]\n", str_length(my_str), my_str);
	}
}
*/

void test() {
	Node *node1 = node_new(Node);
	Node *node2 = node_new(Node);

	reparent(node2, node1);

	ltranslate(node1, vxy(2.0, 0.0));
	set_lrot(node1, 3.14 / 4.0);

	ltranslate(node2, vxy(2.0, 0.0));

	vec2 n2pos = get_gpos(node2);
	printf("pos: %f %f\n", n2pos.x, n2pos.y);
}