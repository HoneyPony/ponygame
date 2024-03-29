#pragma once

#include "pony_clib.h"

#define list_of(Ty) Ty*

uint32_t ls_length(void *list);

void *ls_init_internal(size_t item_size);
void *ls_push_internal(void *list, size_t item_size, void *item_in);
void ls_pop_internal(void *list, size_t item_size, void *item_out);
void ls_delete_internal(void *list, size_t item_size, size_t index);
bool ls_has_internal(void *list, size_t item_size, void *item);
size_t ls_find_internal(void *list, size_t item_size, void *item);
void* ls_prealloc_internal(void *list, size_t item_size, size_t desired_count);

#define ls_prealloc(list, count)\
((list) = ls_prealloc_internal((list), sizeof(*(list)), (count)))

extern void ls_free(void *list);
extern void ls_clear(void *list);

#define ls_init(list)\
	((list) = ls_init_internal(sizeof(*(list))))

#define ls_declare_init(Ty, list)\
	list_of(Ty) list; ls_init(list)
	
#define ls_push(list, item)\
do {\
	typeof(*list) genvar(tmp) = (item);\
	(list) = ls_push_internal((list), sizeof(*list), & genvar(tmp));\
} while(0)

#define ls_push_var(list, var)\
((list) = ls_push_internal((list), sizeof(*list), & var))

#define ls_pop(list)\
({\
	typeof(*list) genvar(result);\
	ls_pop_internal((list), sizeof(*list), & genvar(result));\
	genvar(result);\
})

#define ls_empty(list)\
(ls_length(list) == 0)

#define ls_delete(list, index)\
	ls_delete_internal((list), sizeof(*(list)), (index))

#define ls_has(list, item)\
({\
	typeof(*list) genvar(tmp) = (item);\
	ls_has_internal((list), sizeof(*(list)), & genvar(tmp));\
})

#define ls_find(list, item)\
({\
	typeof(*list) genvar(tmp) = (item);\
	ls_find_internal((list), sizeof(*(list)), & genvar(tmp));\
})

//((*(typeof((list).at))(ls_pushing_ptr(&(list), sizeof(*(list).at)))) = (item))


#define foreach(name, list, ...)\
for(uint32_t genvar(list_index) = 0; genvar(list_index) < ls_length(list); ++genvar(list_index)) {\
	typeof(*list) name = (list)[genvar(list_index)];\
	__VA_ARGS__\
}

#define foreach_r(name, list, ...)\
for(uint32_t genvar(list_index) = 0; genvar(list_index) < ls_length(list); ++genvar(list_index)) {\
	typeof(list) name = &(list)[genvar(list_index)];\
	__VA_ARGS__\
}

#define foreach_l(name, list, length, ...)\
for(size_t genvar(list_index) = 0; genvar(list_index) < length; ++genvar(list_index)) {\
	typeof(*list) name = (list)[genvar(list_index)];\
	__VA_ARGS__\
}
