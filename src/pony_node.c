#include <stdio.h>
#include <SDL2/SDL.h>

#include "pony_node.h"
#include "pony_unsafe_transforms.h"
#include "pony_log.h"

typedef struct NodeLinks Link;
list_of(NodeHeader*) node_header_list;

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

	if(new_previous->next_node) {
		new_previous->next_node->last_node = link_to_insert;
	}

	new_previous->next_node = link_to_insert;
}

static void ensure_free_list_exists(NodeHeader *header) {
	if(header->list_free.next_node == NULL) {
		// Allocate node links in blocks of 64
		// This paremeter could be tuned based on benchmarks, etc...
		//
		// Need to use calloc() so that the links will be set to NULL to
		// start with.
		void *block = pony_calloc(header->node_size, header->alloc_block_size);

		for(size_t i = 0; i < header->alloc_block_size; ++i) {
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
	link_insert_after(&result->internal.links, &header->list_allocated);

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

void *node_new_from_header_unsafe(NodeHeader *header) {
	void *node = node_new_uninit_from_header(header);

	// The header must be stored here as it is not stored elsewhere.
	// TODO: This can be optimized by storing the header when the node is
	// first allocated, as its type will never change.
	((Node*)node)->header = header;

	// Increase the generation when a node is allocated.
	((Node*)node)->internal.generation++;

	// A node becomes valid once allocated.
	((Node*)node)->internal.is_valid = 1;

	node_construct_recursively(node, header);

	if(((Node*)node)->children == NULL) {
		logf_warn("incorrectly constructed node! %p", node);
	}

	return node;
}

void *node_new_from_header(NodeHeader *header) {
	if(header->associated_tree) {
		return header->associated_tree();
	}
	return node_new_from_header_unsafe(header);
}

static void node_destroy_recursive(Node *top) {
	// Free all child trees.
	for(size_t i = 0; i < ls_length(top->children); ++i) {
		node_destroy_recursive(top->children[i]);
	}

	// Free the child list itself.
	ls_free(top->children);
	top->children = NULL;

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

	// Use the last_node pointer in the destroy list to keep track of the very
	// last node in the list. That way, we can move all the nodes from
	// the destroyed list to the free list extremely quickly.
	//
	// We may want to consider renaming last_node to prev_node because this
	// double-use of the word "last" is a bit strange.
	if(top_header->list_destroyed.last_node == NULL) {
		top_header->list_destroyed.last_node = &top->internal.links;
	}

	link_insert_after(&top->internal.links, &top_header->list_destroyed);
	// Finally, the internal metadata in a destroyed node needs to be updated.
	// This will invalidate any Refs to this node, as well as prevent the
	// game loop from executing this node's tick functions.
	top->internal.is_valid = 0;
}

void node_destroy(AnyNode *ptr) {
	if(!ptr) return;

	Node *node = ptr;
	if(node->internal.immortal) return;

	// This node must be detached from the tree.
	reparent(node, NULL);

	node_destroy_recursive(node);
}

void node_header_collect_destroyed_list(NodeHeader *header) {
	Link *d_first = header->list_destroyed.next_node;
	Link *d_last = header->list_destroyed.last_node;

	// NULL list = nothing to collect
	if(header->list_destroyed.next_node == NULL) {
		return;
	}

	// Point the nodes in the destroy list to where they need to be in the
	// free list.
	d_first->last_node = &header->list_free;
	d_last->next_node = header->list_free.next_node;

	// Now that next_node has been tracked by d_last, we can re-point the
	// next_node of the free list.
	header->list_free.next_node = d_first;
	if(d_last->next_node) {
		d_last->next_node->last_node = d_last;
	}

	// Finally, we can completely clear out the destroyed list.
	header->list_destroyed = (Link){ NULL, NULL };
}

void *node_try_downcast_by_header(void *ptr, NodeHeader *header) {
	Node *node = ptr;
	NodeHeader *check = node->header;

	while(check) {
		// If the header to downcast to is in our class heirarchy, the downcast
		// will succeed.
		//
		// Note that downcasting to more specific types is faster than downcasting
		// to less specific types.
		if(check == header) {
			return ptr;
		}
		check = check->base_class;
	}

	// If the type is not found, the downcast fails. Return null in that case,
	// as that allows for nice if block patterns.
	return NULL;
}

uint32_t node_ref_get_generation(void *ref_ptr) {
	if(!ref_ptr) return 0;

	Node *node = ref_ptr;
	return node->internal.generation;
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
	child->internal.matrix_dirty = 1;
}

bool node_ref_is_valid_internal(Node *ptr, uint32_t generation) {
	if(!ptr) return false;
	if(!ptr->internal.is_valid) return false;

	if(ptr->internal.generation != generation) return false;

	return true;
}

void *node_ref_unbox(Node *ptr, uint32_t generation) {
	if(node_ref_is_valid_internal(ptr, generation)) return ptr;
	return NULL;
}

static list_of(Node*) node_process_list = NULL;
Node *root;

void node_add_tree_to_process_list(Node *tree) {
	// Question: do we instead simply want to iterate over all the node headers?
	// That could be faster, as we could skip all nodes that don't have a process
	// function at once.
	//
	// In fact... all the headers that have a process function could simply be
	// added to a list of headers to process...

	NodeHeader *header = tree->header;
	if(header->process) {
		ls_push(node_process_list, tree);
	}

	for(uint32_t i = 0; i < ls_length(tree->children); ++i) {
		node_add_tree_to_process_list(tree->children[i]);
	}
}

void node_compute_transform_tree(Node *tree) {
	node_force_compute_transform(tree);

	for(uint32_t i = 0; i < ls_length(tree->children); ++i) {
		node_compute_transform_tree(tree->children[i]);
	}
}

void node_process_all() {
	foreach(header, node_header_list, {
		node_header_collect_destroyed_list(header);
	})

	if(!node_process_list) {
		ls_init(node_process_list);
	}

uint64_t time0 = SDL_GetTicks64();
	node_add_tree_to_process_list(root);

	uint32_t len = ls_length(node_process_list);
	for(uint32_t i = 0; i < len; ++i) {
		Node *next = node_process_list[i];
		if(next->internal.is_valid) {
			NodeHeader *header = next->header;

			header->process(next, NULL);
		}
	}


//uint64_t time = SDL_GetTicks64();
	node_compute_transform_tree(root);
//time = SDL_GetTicks64() - time;
//printf("time to compute tree: %lu\n", time);

	ls_clear(node_process_list);

	

	uint64_t time1 = SDL_GetTicks64();
	//printf("time to process nodes: %llu -> %llu\n", time0, time1);
}