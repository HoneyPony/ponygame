#include "ponygame.h"
#include <stdio.h>

void test() {
	for(int i = 0; i < 100; ++i) {
		puts("allocating a node");
		fflush(stdout);
		Node *node = new(Node);
		printf("address: %p\n", node);
	}
}