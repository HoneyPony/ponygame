#include "pony_node.h"

#include <stdio.h>

#define find_unset_index(input)\
(__builtin_ctzll(~input))

static void *node_new_from_pool(struct NodePool64 *pool, size_t stride) {
	size_t index = find_unset_index(pool->mask);

	pool->mask |= (1 << index);

	return pool->data + (stride * index);
}

static void *node_new_uninit_from_header(NodeHeader *header) {
	size_t index = header->alloc_last_pool;
	size_t count = header->alloc_pools.length;

	for(size_t counter = 0; counter < count; ++counter) {
		if(~header->alloc_pools.at[index].mask) {
			return node_new_from_pool(&header->alloc_pools.at[index], header->node_size);
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

void *node_new_from_header(NodeHeader *header) {
	void *node = node_new_uninit_from_header(header);

	if(header->construct != NULL) {
		header->construct(node);
	}

	return node;
}