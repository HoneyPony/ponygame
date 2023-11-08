#include "spatial_hash.h"

#include "pony_log.h"

static int32_t
sh_map(int32_t ix, int32_t iy, SpatialHashAABB *aabb) {
	ix -= aabb->min_x;
	iy -= aabb->min_y;

	return iy * (aabb->max_x - aabb->min_x) + ix;
}

static bool
sh_has(int32_t ix, int32_t iy, SpatialHashAABB *aabb) {
	return (ix >= aabb->min_x && ix < aabb->max_x) && (iy >= aabb->min_y && iy < aabb->max_y);
}

static void
sh_resize(SpatialHash *sh, SpatialHashAABB new_aabb) {
	logf_info("sh_resize called");
	size_t total = (new_aabb.max_x - new_aabb.min_x) * (new_aabb.max_y - new_aabb.min_y);

	//printf("total = %zu\n", total);

	list_of(SpatialCell) new_cells;
	ls_init(new_cells);
	ls_prealloc(new_cells, total);

	for(size_t i = 0; i < total; ++i) {
		SpatialCell c = { NULL };
		ls_push(new_cells, c);
	}

	size_t dbg_copied = 0;
	size_t dbg_inited = 0;

	for(int32_t x = new_aabb.min_x; x < new_aabb.max_x; ++x) {
		for(int32_t y = new_aabb.min_y; y < new_aabb.max_y; ++y) {
			// To remap an old cell, we use sh_map.
			//
			// But, if the cell didn't exist, we just make a new cell.
			if(sh_has(x, y, &sh->aabb)) {
				int32_t oldidx = sh_map(x, y, &sh->aabb);
				int32_t newidx = sh_map(x, y, &new_aabb);
				new_cells[newidx] = sh->cells[oldidx];

				dbg_copied += 1;
			}
			else {
				// Make a new cell by initializing that cell's list.
				int32_t newidx = sh_map(x, y, &new_aabb);
				//printf("initializing cell %d %d -> %d\n", x, y, newidx);
				ls_init(new_cells[newidx].nodes);

				dbg_inited += 1;
			}
		}
	}

	logf_info("sh_resize: copied %zu, inited %zu, total = %zu | computed total = %zu", dbg_copied, dbg_inited, (dbg_copied + dbg_inited), total);

	if(sh->cells) {
		ls_free(sh->cells);
	}
	sh->cells = new_cells;
	sh->aabb = new_aabb;
}

void 
sh_init(SpatialHash *sh, NodeHeader *header, float cell_size, int32_t width) {
	sh->header = header;
	sh->cell_size = cell_size;
	sh->cells = NULL;

	sh->aabb.min_x = 0;
	sh->aabb.min_y = 0;
	sh->aabb.max_x = 0;
	sh->aabb.max_y = 0;

	SpatialHashAABB new_aabb = {-width, -width, width + 1, width + 1};
	sh_resize(sh, new_aabb);
}

void
sh_destroy(SpatialHash *sh) {
	/* TODO */
}



void
sh_add(SpatialHash *sh, AnyNode *node, SpatialHashRef *ref) {
	vec2 pos = get_gpos(node);
	int32_t ix = (int32_t)floor(pos.x / sh->cell_size);
	int32_t iy = (int32_t)floor(pos.y / sh->cell_size);

	SpatialHashAABB aabb = sh->aabb;
	bool resize = false;
	if(ix < aabb.min_x) {
		aabb.min_x = ix;
		resize = true;
	}
	if(ix >= aabb.max_x) {
		aabb.max_x = ix + 1;
		resize = true;
	}
	if(iy < aabb.min_y) {
		aabb.min_y = iy;
		resize = true;
	}
	if(iy >= aabb.max_y) {
		aabb.max_y = iy + 1;
		resize = true;
	}

	if(resize) {
		sh_resize(sh, aabb);
	}

	// At this point, there are enough cells, we can directly put it in
	// a cell.
	//
	// TODO: Make cells hash sets...?
	int32_t idx = sh_map(ix, iy, &sh->aabb);
	ls_push(sh->cells[idx].nodes, node);

	ref->x = ix;
	ref->y = iy;
}

void
sh_remove(SpatialHash *sh, AnyNode *node, SpatialHashRef *ref) {
	if(sh_has(ref->x, ref->y, &sh->aabb)) {
		int32_t idx = sh_map(ref->x, ref->y, &sh->aabb);
		SpatialCell *sc = &sh->cells[idx];

		// Search for the node.
		size_t to_del = ls_find(sc->nodes, node);

		// O(1) removal by popping and swapping.
		AnyNode *swap = ls_pop(sc->nodes);
		sc->nodes[to_del] = swap;

		ref->x = -0x80000000;
		ref->y = -0x80000000;
	}
}

void
sh_update(SpatialHash *sh, AnyNode *node, SpatialHashRef *ref) {
	vec2 pos = get_gpos(node);
	int32_t ix = (int32_t)floor(pos.x / sh->cell_size);
	int32_t iy = (int32_t)floor(pos.y / sh->cell_size);

	// Don't both doing the whole search-remove-re-add song and dance
	// if it's unecessary.
	if(ix == ref->x && iy == ref->y) return;

	sh_remove(sh, node, ref);
	sh_add(sh, node, ref);
}

void
sh_find(SpatialHash *sh, vec2 center, float radius, list_of(AnyNode*) *output_buffer) {
	int32_t min_x = (int32_t)floor((center.x - radius) / sh->cell_size);
	int32_t min_y = (int32_t)floor((center.y - radius) / sh->cell_size);
	int32_t max_x = (int32_t)ceil((center.x + radius) / sh->cell_size);
	int32_t max_y = (int32_t)ceil((center.y + radius) / sh->cell_size);

	if(min_x < sh->aabb.min_x) min_x = sh->aabb.min_x;
	if(min_y < sh->aabb.min_y) min_y = sh->aabb.min_y;

	// exclusive converted to inclusive
	if(max_x >= sh->aabb.max_x) max_x = sh->aabb.max_x - 1;
	if(max_y >= sh->aabb.max_y) max_y = sh->aabb.max_y - 1;

	for(int32_t x = min_x; x <= max_x; ++x) {
		for(int32_t y = min_y; y <= max_y; ++y) {
			SpatialCell *sc = &sh->cells[sh_map(x, y, &sh->aabb)];

			foreach(node, sc->nodes, {
				vec2 pos = get_gpos(node);
				vec2 dist = sub(pos, center);
				if(dot(dist, dist) <= (radius * radius)) {
					ls_push(*output_buffer, node);
				}
			})
		}
	}
}

AnyNode*
sh_find_one(SpatialHash *sh, vec2 center, float radius) {
	int32_t min_x = (int32_t)floor((center.x - radius) / sh->cell_size);
	int32_t min_y = (int32_t)floor((center.y - radius) / sh->cell_size);
	int32_t max_x = (int32_t)ceil((center.x + radius) / sh->cell_size);
	int32_t max_y = (int32_t)ceil((center.y + radius) / sh->cell_size);

	if(min_x < sh->aabb.min_x) min_x = sh->aabb.min_x;
	if(min_y < sh->aabb.min_y) min_y = sh->aabb.min_y;

	// exclusive converted to inclusive
	if(max_x >= sh->aabb.max_x) max_x = sh->aabb.max_x - 1;
	if(max_y >= sh->aabb.max_y) max_y = sh->aabb.max_y - 1;

	for(int32_t x = min_x; x <= max_x; ++x) {
		for(int32_t y = min_y; y <= max_y; ++y) {
			SpatialCell *sc = &sh->cells[sh_map(x, y, &sh->aabb)];

			foreach(node, sc->nodes, {
				vec2 pos = get_gpos(node);
				vec2 dist = sub(pos, center);
				if(dot(dist, dist) <= (radius * radius)) {
					return node;
				}
			})
		}
	}

	return NULL;
}

AnyNode*
sh_find_closest(SpatialHash *sh, vec2 center, float radius) {
	int32_t min_x = (int32_t)floor((center.x - radius) / sh->cell_size);
	int32_t min_y = (int32_t)floor((center.y - radius) / sh->cell_size);
	int32_t max_x = (int32_t)ceil((center.x + radius) / sh->cell_size);
	int32_t max_y = (int32_t)ceil((center.y + radius) / sh->cell_size);

	if(min_x < sh->aabb.min_x) min_x = sh->aabb.min_x;
	if(min_y < sh->aabb.min_y) min_y = sh->aabb.min_y;

	// exclusive converted to inclusive
	if(max_x >= sh->aabb.max_x) max_x = sh->aabb.max_x - 1;
	if(max_y >= sh->aabb.max_y) max_y = sh->aabb.max_y - 1;

	AnyNode *closest = NULL;
	float closest_dist = 0.f;

	for(int32_t x = min_x; x <= max_x; ++x) {
		for(int32_t y = min_y; y <= max_y; ++y) {
			SpatialCell *sc = &sh->cells[sh_map(x, y, &sh->aabb)];

			foreach(node, sc->nodes, {
				vec2 pos = get_gpos(node);
				vec2 dist_vec = sub(pos, center);
				float dist = dot(dist_vec, dist_vec);
				if(closest) {
					if(dist < closest_dist) {
						closest = node;
						closest_dist = dist;
					}
				}
				else if(dist <= (radius * radius)) {
					closest = node;
					closest_dist = dist;
				}
			})
		}
	}

	return closest;
}