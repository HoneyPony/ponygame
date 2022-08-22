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

#define BENCH_SIZE (1024 * 1024 * 16)

void bench_node_construct_destruct() {
	ty_init(1, Node)
	ty_init(2, Ty1)
	ty_init(3, Ty2)
	ty_init(4, Ty3)
	ty_init(5, Ty4)
	ty_init(6, Ty5)
	ty_init(7, Ty6)
	ty_init(8, Ty7)

	clock_t start, end;

	Ty8 **ptrs = pony_malloc(sizeof(*ptrs) * BENCH_SIZE);

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
	// Step through the nodes in a fashion that will cause them to be linked
	// in a jump way when we re-create them.
	for(size_t j = 0; j < 256; ++j) {
		for(size_t i = 0; i < BENCH_SIZE / 256; i += 1) {
			node_destroy(ptrs[j + (i * 256)]);
		}
	}
	/*for(size_t i = 0; i < BENCH_SIZE; ++i) {
		node_destroy(ptrs[i]);
	}*/
	end = clock();

	double time_destroy = (end - start) / (double)CLOCKS_PER_SEC;
	double time_per_destroy_op = (time_destroy / BENCH_SIZE) * 1000000.0;

	node_header_collect_destroyed_list(&node_header(Node));
	puts("benchmarking node re-creation, with more jumbled list.");
	start = clock();
	for(size_t i = 0; i < BENCH_SIZE; ++i) {
		ptrs[i] = node_new(Ty8);
	}
	end = clock();

	double time_create_again = (end - start) / (double)CLOCKS_PER_SEC;
	double time_per_create_again_op = (time_create_again / BENCH_SIZE) * 1000000.0;

	printf("results:\n"
		"\tnode create  = %f us\n"
		"\tnode destroy = %f us\n"
		"\tnode create again = %f us\n",
		time_per_create_op,
		time_per_destroy_op,
		time_per_create_again_op);
}

static void clear_node_alloc_info() {
	NodeHeader *h = &node_header(Ty8);

	h->list_allocated = (struct NodeInternal){ NULL, NULL, 0, 0 };
	h->list_destroyed = (struct NodeInternal){ NULL, NULL, 0, 0 };
	h->list_free = (struct NodeInternal){ NULL, NULL, 0, 0 };
}

#define BATCH_SIZE 512
#define BATCH_COUNT 1024 * 32

void bench_node_construct_destruct_small_batches() {
	ty_init(1, Node)
	ty_init(2, Ty1)
	ty_init(3, Ty2)
	ty_init(4, Ty3)
	ty_init(5, Ty4)
	ty_init(6, Ty5)
	ty_init(7, Ty6)
	ty_init(8, Ty7)

	clock_t start, end;

	Ty8 **ptrs = pony_malloc(sizeof(*ptrs) * BATCH_SIZE);

	puts("benchmark: general node ops, in batches...");

	start = clock();

	for(size_t batch = 0; batch < BATCH_COUNT; ++batch) {
		for(size_t i = 0; i < BATCH_SIZE; ++i) {
			ptrs[i] = node_new(Ty8);
		}
		// Step through the nodes in a fashion that will cause them to be linked
		// in a jump way when we re-create them.
		for(size_t j = 0; j < 256; ++j) {
			for(size_t i = 0; i < BATCH_SIZE / 256; i += 1) {
				node_destroy(ptrs[j + (i * 256)]);
			}
		}

		node_header_collect_destroyed_list(&node_header(Node));

		for(size_t i = 0; i < BATCH_SIZE; ++i) {
			ptrs[i] = node_new(Ty8);
		}

		clear_node_alloc_info();
	}

	end = clock();

	size_t total_ops = BATCH_SIZE * BATCH_COUNT * 3;
	double time = (end - start) / (double)(CLOCKS_PER_SEC);
	double time_per_op_us = time * (1000000.0 / total_ops);

	printf("average time per op: %f us\n"
	"note: this time is not particularly representative of a given single operation.\n"
	"      it is known from the other benchmark that in-order deletion is faster than\n"
	"      in-order allocation, but that out-of-order deletion is very slow.\n\n"
	"      this benchmark is primarily to measure whether operations seem faster or\n"
	"      slower, from a very zoomed out perspective, when timed in smaller batches.",
	
	time_per_op_us);
}