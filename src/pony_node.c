#include "pony_node.h"

#include <stdio.h>

typedef struct NodeIntrusiveLinks Link;

static void link_unlink(Link *target) {
	if(!target) return;

	if(target->last_node) {
		target->last_node->next_node = target->next_node;
	}

	if(target->next_node) {
		target->next_node->last_node = target->last_node;
	}

	target->last_node = NULL;
	target->next_node = NULL;
}

static void link_insert_after(Link *link_to_insert, Link *new_previous) {
	link_unlink(link_to_insert);

	link_to_insert->next_node = new_previous->next_node;
	link_to_insert->last_node = new_previous;

	new_previous->next_node = link_to_insert;
}

static void ensure_free_list_exists(NodeHeader *header) {
	if(header->list_free.next_node == NULL) {
		// Allocate node links in blocks of 64
		// This paremeter could be tuned based on benchmarks, etc...
		//
		// Need to use calloc() so that the links will be set to NULL to
		// start with.
		void *block = pony_calloc(header->node_size, 64);

		for(size_t i = 0; i < 64; ++i) {
			// When we call link_insert_after, this will cause all the nodes
			// to be laid out in reverse order in memory.
			//
			// I believe this is generally fine because if there are two
			// blocks allocated very close to each other, this will ensure
			// better cache friendliness for the first allocation, which
			// is the only one we can really control at this point.
			size_t offset = i * header->node_size;

			Link *link = (Link*)(block + offset);
			link_insert_after(link, &header->list_free);
		}
	}
}

static void *node_new_uninit_from_header(NodeHeader *header) {
	// Need to make sure there is at least one entry in the free list.
	ensure_free_list_exists(header);

	Node *result = (Node*)header->list_free.next_node;
	link_insert_after(&result->alloc_info, &header->list_allocated);

	return result;
}

// In order to construct a Node, we essentially need to follow the linked list
// in reverse. This is easily accomplished with a recursive stack that inverts
// the list.
//
// This should probably be benchmarked to see if it is really slow.
void node_construct_recursively(void *node, NodeHeader *next) {
	// Terminate when there is no class.
	if(next == NULL) return;

	// First step: call base class constructor.
	node_construct_recursively(node, next->base_class);

	// Second step: call this constructor (if it exists).
	if(next->construct != NULL) {
		next->construct(node);
	}
}

void *node_new_from_header(NodeHeader *header) {
	void *node = node_new_uninit_from_header(header);

	// The header must be stored here as it is not stored elsewhere.
	// TODO: This can be optimized by storing the header when the node is
	// first allocated, as its type will never change.
	((Node*)node)->header = header;

	node_construct_recursively(node, header);

	return node;
}

static void node_destroy_recursive(Node *top) {
	// Free all child trees.
	for(size_t i = 0; i < ls_length(top->children); ++i) {
		node_destroy_recursive(top->children[i]);
	}

	// Free the child list itself.
	ls_free(top->children);

	// Iterate through the class heirarchy to call destructors.
	NodeHeader *destruct_header = top->header;
	while(destruct_header) {
		if(destruct_header->destruct) {
			destruct_header->destruct(top);
		}
		destruct_header = destruct_header->base_class;
	}

	// Now, the node cannot yet be placed in the free list. It will now be
	// placed in the destroyed list, and can be moved to the free list at
	// the end of the frame.
	//
	// IMPORTANT: This must be the header for *this node*, not the header
	// used for recursing.
	NodeHeader *top_header = top->header;
	link_insert_after(&top->alloc_info, &top_header->list_free);
}

void node_destroy(AnyNode *ptr) {
	Node *node = ptr;

	// This node must be detached from the tree.
	reparent(node, NULL);

	node_destroy_recursive(node);
}

/* Some slighlty less internal functions... */

void reparent(AnyNode *child_ptr, AnyNode *new_parent_ptr) {
	Node *child = child_ptr;
	Node *new_parent = new_parent_ptr;

	if(child->parent) {
		// Assumed invariant: Any child->parent ptr MUST have that child in
		// its 'children' list.
		size_t index = ls_find(child->parent->children, child);
		ls_delete(child->parent->children, index);

		child->parent = NULL;
	}

	child->parent = new_parent;
	if(new_parent) {
		ls_push(new_parent->children, child);
	}

	// TODO: Use matrix inverse to properly re-position, etc, child.
	child->raw_tform.matrix_dirty = 1;
}