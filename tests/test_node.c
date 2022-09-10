#include "test.h"

#include "pony_node.h"

#define FieldList_Child \
FieldList_Node \
int child;

#define FieldList_Grandchild \
FieldList_Child \
int grandchild;

#define FieldList_UnrelatedChild \
FieldList_Node \
int unrelated_child;

#define FieldList_ConstructDestruct \
FieldList_Node \
int int_var; \
list_of(int) list_var;

node_from_field_list(Child)
node_from_field_list(Grandchild)
node_from_field_list(UnrelatedChild)
node_from_field_list(ConstructDestruct)

node_meta_defines(Child)
node_meta_defines(Grandchild)
node_meta_defines(UnrelatedChild)
node_meta_defines(ConstructDestruct)

#define TEST_CONSTRUCT_INT_VAL 10
#define TEST_DESTRUCT_INT_VAL 20
#define TEST_CONSTRUCT_LIST_LENGTH 5

void construct_ConstructDestruct(void *ptr) {
	ConstructDestruct *self = ptr;

	self->int_var = TEST_CONSTRUCT_INT_VAL;
	ls_init(self->list_var);

	for(int i = 0; i < TEST_CONSTRUCT_LIST_LENGTH; ++i) {
		ls_push(self->list_var, i * 2);
	}
}

void destruct_ConstructDestruct(void *ptr) {
	ConstructDestruct *self = ptr;

	self->int_var = TEST_DESTRUCT_INT_VAL;
	ls_free(self->list_var);
	self->list_var = NULL;
}

test_define(test_node_constructor, {
	ConstructDestruct *cd = new(ConstructDestruct);
	test_assert_eq(TEST_CONSTRUCT_INT_VAL, cd->int_var, "Expected var initialized to %d, got %d\n");

	if(ls_length(cd->list_var) != TEST_CONSTRUCT_LIST_LENGTH) {
		test_assert(0, "List initialized with incorrect length");
	}
	else {
		for(uint32_t i = 0; i < ls_length(cd->list_var); ++i) {
			test_assert_eq(i * 2, (uint32_t)cd->list_var[i], "Expected list member initialized to %d, got %d\n");
		}
	}
})

test_define(test_node_destructor, {
	ConstructDestruct *cd = new(ConstructDestruct);
	node_destroy(cd);

	test_assert_eq(TEST_DESTRUCT_INT_VAL, cd->int_var, "Expected var initialized to %d, got %d\n");
	test_assert(cd->list_var == NULL, "Expected null list var, got %p\n", cd->list_var);
})

test_define(test_node_downcast, {
	Child *test = new(Child);
	void *throwaway = test;

	Child *downcast = node_try_downcast_by_header(throwaway, &node_header(Child));
	test_assert(test == downcast, "Expected downcast to succeed. Instead, got %p for downcast.\n", downcast);
})

test_define(test_node_downcast_grandchild, {
	Grandchild *test = new(Grandchild);
	void *throwaway = test;

	Grandchild *downcast = node_try_downcast_by_header(throwaway, &node_header(Grandchild));
	test_assert(test == downcast, "Expected downcast to succeed. Instead, got %p for downcast.\n", downcast);
})

test_define(test_node_downcast_partway, {
	Grandchild *test = new(Grandchild);
	void *throwaway = test;

	Child *downcast = node_try_downcast_by_header(throwaway, &node_header(Child));
	test_assert(downcast && ((void*)downcast == (void*)test), "Expected partial downcast to succeed, but got %p\n", downcast);
})

test_define(test_node_cant_downcast, {
	UnrelatedChild *child = new(UnrelatedChild);
	void *throwaway = child;

	Child *failed_downcast = node_try_downcast_by_header(throwaway, &node_header(Child));
	test_assert(!failed_downcast, "Expected failed downcast, but got %p\n", failed_downcast);
})

test_define(test_node_using_node, {
	Child *test = new(Child);
	void *node = test;

	Child *child;
	using_node(node as Child as child) {
		test_assert(child->header == &node_header(Child), "Bad downcast in using block.");
	}
	else {
		test_assert(0, "Using block incorrectly return null.");
	}
})

test_set_define(test_set_node, {
	// Need some setup for this test set.
	node_meta_initialize(Child, &node_header(Node), NULL, NULL, NULL, BLOCKS_TINY);
	node_meta_initialize(Grandchild, &node_header(Child), NULL, NULL, NULL, BLOCKS_TINY);
	node_meta_initialize(UnrelatedChild, &node_header(Node), NULL, NULL, NULL, BLOCKS_TINY);
	node_meta_initialize(ConstructDestruct, &node_header(Node),
		construct_ConstructDestruct,
		NULL,
		destruct_ConstructDestruct,
		BLOCKS_TINY)

	test_node_constructor();
	test_node_destructor();

	test_node_downcast();
	test_node_downcast_grandchild();
	test_node_downcast_partway();
	test_node_cant_downcast();
	test_node_using_node();
})
