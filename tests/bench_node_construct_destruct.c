#include "test.h"

#include "pony_node.h"

// TODO: Write a couple helper methods for benchmarks...?
#include <time.h>

#define FieldList_Ty1 \
FieldList_Node \
int ty1;

#define FieldList_Ty2 \
FieldList_Ty1 \
int ty2;

#define FieldList_Ty3 \
FieldList_Ty2 \
int ty3;

#define FieldList_Ty4 \
FieldList_Ty3 \
int ty4;

#define FieldList_Ty5 \
FieldList_Ty4 \
int ty5;

#define FieldList_Ty6 \
FieldList_Ty5 \
int ty6;

#define FieldList_Ty7 \
FieldList_Ty6 \
int ty7;

#define FieldList_Ty8 \
FieldList_Ty7 \
int ty8;

node_from_field_list(Ty1)
node_from_field_list(Ty2)
node_from_field_list(Ty3)
node_from_field_list(Ty4)
node_from_field_list(Ty5)
node_from_field_list(Ty6)
node_from_field_list(Ty7)
node_from_field_list(Ty8)

#define toplevel(num)\
void construct_Ty ## num (void *ptr) {\
	Ty ## num *self = ptr;\
	self->ty ## num = num * 2;\
}\
\
void destruct_Ty ## num (void *ptr) {\
	Ty ## num *self = ptr;\
	self->ty ## num *= 3;\
}\
\
node_meta_defines(Ty ## num)

#define ty_init(num, last)\
node_meta_initialize(Ty ## num, &node_header(last), construct_Ty ## num, NULL, destruct_Ty ## num)

toplevel(1)
toplevel(2)
toplevel(3)
toplevel(4)
toplevel(5)
toplevel(6)
toplevel(7)
toplevel(8)

#define BENCH_SIZE (1024 * 1024 * 4)

Node *ptrs[BENCH_SIZE];

void bench_node_construct_destruct() {
	ty_init(1, Node)
	ty_init(2, Ty1)
	ty_init(3, Ty2)
	ty_init(4, Ty3)
	ty_init(5, Ty4)
	ty_init(6, Ty5)
	ty_init(7, Ty6)
	ty_init(8, Ty7)

	printf("destruct ptr for Ty 8: %x\n\t(inside %x)\n", node_header(Ty8).destruct, &node_header(Ty8));

	clock_t start, end;

	puts("benchmarking node creation.");
	start = clock();
	for(size_t i = 0; i < BENCH_SIZE; ++i) {
		ptrs[i] = node_new(Ty8);
	}
	end = clock();

	double time_create = (end - start) / (double)CLOCKS_PER_SEC;
	double time_per_create_op = (time_create / BENCH_SIZE) * 1000000.0;

	puts("benchmarking node destruction.");
	start = clock();
	for(size_t i = 0; i < BENCH_SIZE; ++i) {
		node_destroy(ptrs[i]);
	}
	end = clock();

	double time_destroy = (end - start) / (double)CLOCKS_PER_SEC;
	double time_per_destroy_op = (time_destroy / BENCH_SIZE) * 1000000.0;

	printf("results:\n\tnode create  = %f us\n\tnode destroy = %f us\n", time_per_create_op, time_per_destroy_op);
}