#include "pony_node.h"

#include <stdio.h>

#define find_unset_index(input)\
(__builtin_ctzll(~input))

static void *node_new_from_pool(struct NodePool64 *pool, size_t stride) {
	size_t index = find_unset_index(pool->mask);

	pool->mask |= (1 << index);

	void *result_ptr = pool->data + (stride * index);
	Node *node = result_ptr;

	// Need to track pool for freeing purposes.
	node->source_pool = pool;
	return result_ptr;
}

static void *node_new_uninit_from_header(NodeHeader *header) {
	size_t index = header->alloc_last_pool;
	size_t count = ls_length(header->alloc_pools);

	for(size_t counter = 0; counter < count; ++counter) {
		if(~header->alloc_pools[index].mask) {
			return node_new_from_pool(&header->alloc_pools[index], header->node_size);
		}

		index = (index + 1) % count;
	}

	struct NodePool64 new_pool;
	new_pool.data = pony_malloc(header->node_size * 64);
	new_pool.mask = 0;

	void *result = node_new_from_pool(&new_pool, header->node_size);

	ls_push(header->alloc_pools, new_pool);

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

	node_construct_recursively(node, header);

	return node;
}

void node_free_in_pool(Node *node, struct NodePool64 *pool, size_t stride) {
	uint8_t *ptr = (uint8_t*)(node);
	size_t index = (ptr - pool->data) / stride;
	pool->mask &= ~(1 << index);
}

/*
void node_free_in_header(Node *node, NodeHeader *header) {
	// Linear search of pools...
	for(size_t i = 0; i < ls_length(header->alloc_pools); ++i) {
		struct NodePool64 *pool = &header->alloc_pools[i];

		if(node >= pool->data) {
			if(node < pool->data + 64 * header->node_size) {
				node_free_in_pool(node, pool, header->node_size);
				return;
			}
		}
	}
}
*/

static void node_free(Node *node) {
	node_free_in_pool(node, node->source_pool, node->header->node_size);
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

static void node_free_recursive(Node *top) {
	// Free all child trees.
	for(size_t i = 0; i < ls_length(top->children); ++i) {
		node_free_recursive(top->children[i]);
	}

	// Free the child list itself.
	ls_free(top->children);

	NodeHeader *header = top->header;
	while(header) {
		if(header->destruct) {
			header->destruct(top);
		}
		header = header->base_class;
	}

	// Free this node object.
	node_free(top);

	// TODO: Handle possible cycles? Seems mostly pointless. It would be
	// very difficult to accidentally create a cycle.
	// Like, the only way to create a cycle is to *detach* a node from the tree,
	// and *then* create a cycle. So IDK. Maybe.
}

void node_destroy(AnyNode *ptr) {
	Node *node = ptr;

	// This node must be detached from the tree.
	reparent(node, NULL);

	node_free_recursive(node);
}