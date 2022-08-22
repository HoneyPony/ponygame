#include "test.h"

#include "pony_node.h"

test_define(test_node_ref_deletion, {
	Node *node = new(Node);

	Ref(Node) my_ref = ref(node);

	test_assert(valid(my_ref), "Reference should be valid when created.");

	node_destroy(node);

	test_assert(!valid(my_ref), "Reference should not be valid after destroying node.");
})

// Note: this test current doesn't work because structs with the same structure
// cannot actually be copied with = unless they are the actual same type.
//
// There are several ways the API could be changed to make this better... maybe
// all refs could be the same type? Then of course they would have to be
// specially cast whenever used. But that might be fine, as it would basically
// force you to check that they're valid at that moment.
//
// (That would technically be a little inefficient, but I think it would
// generally be fine...?)
/*test_define(test_node_ref_copy, {
	Node *node = new(Node);

	Ref(Node) first = ref(node);
	Ref(Node) second = first;

	test_assert(valid(second), "Reference should be valid when created.");

	node_destroy(node);

	test_assert(!valid(second), "Copied reference should not be valid after destroying node.");
})*/

test_define(test_node_ref_generation, {
	// Need to collect destroyed nodes first so that we can create and free
	// a single node and expect it to be at the front of the list.
	node_header_collect_destroyed_list(&node_header(Node));

	Node *node = new(Node);

	Ref(Node) the_ref = ref(node);
	
	node_destroy(node);

	// Destroyed nodes must be collected again to put that node at the beginning
	// of the free list.
	node_header_collect_destroyed_list(&node_header(Node));

	Node *second = new(Node);

	if(node == second) {
		puts("Note: Successfully testing references of same node with different generation.");
		test_assert(!valid(the_ref), "Reference to same node with different generation should be invalid.");
	}
	else {
		puts("Note: Generation test did not create two identical node pointers.");
		test_assert(0, "Check if this test can be updated to work with the current allocation scheme.");
	}
})

test_set_define(test_set_node_ref, {
	test_node_ref_deletion();
	//test_node_ref_copy();
	test_node_ref_generation();
})