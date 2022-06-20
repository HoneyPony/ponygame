#pragma once

#include "pony_clib.h"

#define list_of(Ty)\
struct { int32_t alloc; int32_t length; Ty *at; }

void *ls_pushing_ptr(void *list_ptr, size_t item_size);
void *ls_popping_ptr(void *list_ptr, size_t item_size);
void ls_delete_thru_size(void *list_ptr, size_t item_size, size_t index);

#define ls_push(list, item)\
((*(typeof((list).at))(ls_pushing_ptr(&(list), sizeof(*(list).at)))) = (item))

#define ls_pop(list)\
(*(typeof((list).at)*)(ls_popping_ptr(&(list), sizeof(*(list).at))))

#define ls_is_empty(list)\
((list).length == 0)

#define ls_delete(list, index)\
ls_delete_thru_size(&(list), sizeof((list).at), index)

#define ls_initialize(list_var)\
do { list_var.alloc = 0; list_var.length = 0; list_var.at = NULL; } while(0)
