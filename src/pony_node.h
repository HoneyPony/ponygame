#pragma once

#include "pony_field_list.h"
#include "pony_clib.h"
#include "pony_list.h"

#include "pony_transform.h"

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


extern void *node_new_from_header(NodeHeader *header);

#define node_new(Ty) ((Ty*)node_new_from_header(&node_header(Ty)))
#define new(Ty) node_new(Ty)



/* The raw Node type. This type is responsible for a lot of stuff, including
 * both the basic scene tree AND the transform heirarchy (as they are extremely
 * interconnected in this game engine.
 */

#define FieldList_Node \
NodeHeader *header; \
struct Node *parent; \
list_of(struct Node*) children; \
RawTransform raw_tform;

node_from_field_list(Node)

typedef void AnyNode;

vec2 get_lpos(AnyNode *node);
vec2 get_gpos(AnyNode *node);
vec2 get_lscale(AnyNode *node);
//vec2 get_gscale(void *node);
float get_lrot(AnyNode *node);
float get_grot(AnyNode *node);
float get_lrot_deg(AnyNode *node);
float get_grot_deg(AnyNode *node);

void set_lpos(AnyNode *node, vec2 pos);
void set_gpos(AnyNode *node, vec2 pos);
void set_lscale(AnyNode *node, vec2 scale);
//vec2 set_gscale(void *node, vec2 scale);
void set_lrot(AnyNode *node, float rad);
void set_grot(AnyNode *node, float rad);
void set_lrot_deg(AnyNode *node, float deg);
void set_grot_deg(AnyNode *node, float deg);

vec2 get_basis_x(AnyNode *node);
vec2 get_basis_y(AnyNode *node);

vec2 local_to_global(AnyNode *node, vec2 local);
vec2 global_to_local(AnyNode *node, vec2 global);

const RawTransform *node_get_parent_transform(Node *node);
bool node_update_transform(Node *node);

void reparent(AnyNode *child, AnyNode *new_parent);