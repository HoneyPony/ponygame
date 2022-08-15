#pragma once

#include "pony_field_list.h"
#include "pony_clib.h"
#include "pony_list.h"

struct Node;

struct NodePool64 {
	uint8_t *data;
	uint64_t mask;
};

typedef void (*NodeConstructor)(void *node);
typedef void (*NodeProcess)(void *node, void *tree);

typedef struct NodeHeader {
	struct NodeHeader *base_class;

	size_t node_size;

	list_of(struct NodePool64) alloc_pools;
	size_t alloc_last_pool;

	NodeConstructor construct;
	NodeProcess process;
} NodeHeader;

#define node_header(Ty) macro_concat(node_header_for_, Ty)

#define node_from_field_list(List) \
struct_from_field_list(List) \
extern NodeHeader node_header(List) ;

#define node_meta_initialize(Ty, base_class_ptr, construct_f, process_f) \
node_header(Ty).base_class = base_class_ptr; \
node_header(Ty).node_size = sizeof(Ty); \
ls_init(node_header(Ty).alloc_pools); \
node_header(Ty).alloc_last_pool = 0; \
node_header(Ty).construct = construct_f; \
node_header(Ty).process = process_f;

#define node_meta_defines(Ty)\
NodeHeader node_header(Ty);

#define FieldList_Node \
NodeHeader *header; \
struct Node *parent; \
list_of(struct Node*) children;

node_from_field_list(Node)

extern void *node_new_from_header(NodeHeader *header);

#define node_new(Ty) ((Ty*)node_new_from_header(&node_header(Ty)))
#define new(Ty) node_new(Ty)