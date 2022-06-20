#pragma once

#define struct_from_field_list(List) \
typedef struct List { \
	FieldList_ ## List \
} List ;
