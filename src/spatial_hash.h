#ifndef PONY_SPATIAL_HASH_H
#define PONY_SPATIAL_HASH_H

#include "pony_node.h"

typedef struct {
	list_of(AnyNode*) nodes;
} SpatialCell;

typedef struct {
	int32_t min_x;
	int32_t min_y;
	int32_t max_x; // The maximums are exclusive.
	int32_t max_y;
} SpatialHashAABB;

typedef struct {
	NodeHeader *header;
	list_of(SpatialCell) cells;
	
	float cell_size;

	SpatialHashAABB aabb;
} SpatialHash;

typedef struct {
	int32_t x;
	int32_t y;
} SpatialHashRef;

void sh_init(SpatialHash *sh, NodeHeader *header, float cell_size, int32_t width);
void sh_destroy(SpatialHash *sh);

void sh_add(SpatialHash *sh, AnyNode *node, SpatialHashRef *ref);
void sh_remove(SpatialHash *sh, AnyNode *node, SpatialHashRef *ref);
void sh_update(SpatialHash *sh, AnyNode *node, SpatialHashRef *ref);

/** Return true if this node may be selected, false otherwise. */
typedef int (*sh_filter_fn)(AnyNode *node, void *user_data);

typedef struct {
	sh_filter_fn fn;
	void *user_data;
} sh_filter;

void sh_find(SpatialHash *sh, vec2 center, float radius, list_of(AnyNode*) *output_buffer);
AnyNode *sh_find_one(SpatialHash *sh, vec2 center, float radius);
AnyNode *sh_find_closest(SpatialHash *sh, vec2 center, float radius, sh_filter filter);

#endif