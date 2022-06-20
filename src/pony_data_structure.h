#pragma once

#include "pony_clib.h"
#include "pony_object.h"

struct DSPrefix {
	uint32_t alloc;
	uint32_t length;
};

obj_assert_prefix_type(struct DSPrefix)

extern void *ds_list_new_from_size(size_t item_size);

extern void *ds_push_thru_ptr(void *ds, size_t item_size, void *item);
extern void ds_delete_thru_size(void *ds, size_t item_size, size_t index);
extern void ds_pop_thru_ptr(void *ds, size_t item_size, void *out);

extern bool ds_has_thru_ptr(void *ds, size_t item_size, void *item_to_compare);

extern uint32_t ds_length(void *ds);

#define ds_init_list(ds)\
	((ds) = ds_list_new_from_size(sizeof(*(ds))))

#define ds_push(ds, item)\
do {\
	typeof(*ds) genvar(tmp) = (item);\
	(ds) = ds_push_thru_ptr((ds), sizeof(*ds), & genvar(tmp));\
} while(0)

#define ds_push_var(ds, var)\
((ds) = ds_push_thru_ptr((ds), sizeof(*ds), & var))

#define ds_pop(ds)\
({\
	typeof(*ds) genvar(result);\
	ds_pop_thru_ptr((ds), sizeof(*ds), & genvar(result));\
	genvar(result);\
})

#define ds_empty(ds)\
(ds_length(ds) == 0)

#define ds_delete(ds, index)\
	ds_delete_thru_size((ds), sizeof(*(ds)), (index))

#define ds_has(ds, item)\
({\
	typeof(*ds) genvar(tmp) = (item);\
	ds_has_thru_ptr((ds), sizeof(*(ds)), & genvar(tmp));\
})

#define ds_list_of(Ty) Ty*