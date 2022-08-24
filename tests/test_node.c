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

node_from_field_list(Child)
node_from_field_list(Grandchild)
node_from_field_list(UnrelatedChild)

node_meta_defines(Child)
node_meta_defines(Grandchild)
node_meta_defines(UnrelatedChild)

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

test_set_define(test_set_node, {
	// Need some setup for this test set.
	node_meta_initialize(Child, &node_header(Node), NULL, NULL, NULL);
	node_meta_initialize(Grandchild, &node_header(Child), NULL, NULL, NULL);
	node_meta_initialize(UnrelatedChild, &node_header(Node), NULL, NULL, NULL);

	test_node_downcast();
	test_node_downcast_grandchild();
	test_node_downcast_partway();
	test_node_cant_downcast();
})
