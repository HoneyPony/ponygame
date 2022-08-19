#pragma once

#include "pony_field_list.h"
#include "pony_clib.h"
#include "pony_list.h"

#include "pony_transform.h"

struct Node;

struct NodeIntrusiveLinks {
	struct NodeIntrusiveLinks *last_node;
	struct NodeIntrusiveLinks *next_node;
};

typedef void (*NodeConstructor)(void *node);
typedef void (*NodeProcess)(void *node, void *tree);

typedef struct NodeHeader {
	struct NodeHeader *base_class;

	size_t node_size;

	// The linked list of nodes of this type that are currently allocated.
	// When a node is allocated, it is added to this list.
	struct NodeIntrusiveLinks list_allocated;

	// The linked list of nodes that have been destroyed. These nodes are not
	// processed in the current frame. However, they are also not available
	// for allocation, as there may still be references to them in the node
	// process list. They will be made available for allocation at the end
	// of the frame.
	struct NodeIntrusiveLinks list_destroyed;

	// The linked list of nodes that are available for allocation. If this
	// list is completely empty, a new block of nodes will be allocated and
	// linked together. This is to encourage some cache-friendly behavior,
	// although the cache-friendliness will likely decrease over time as
	// nodes from disparate blocks get linked together through multiple
	// de-allocations and re-allocations.
	struct NodeIntrusiveLinks list_free;

	NodeConstructor construct;
	NodeProcess process;
	NodeConstructor destruct;
} NodeHeader;

#define node_header(Ty) macro_concat(node_header_for_, Ty)

#define node_from_field_list(List) \
struct_from_field_list(List) \
extern NodeHeader node_header(List) ;

#define node_meta_initialize(Ty, base_class_ptr, construct_f, process_f, destruct_f) \
node_header(Ty).base_class = base_class_ptr; \
node_header(Ty).node_size = sizeof(Ty); \
node_header(Ty).list_allocated = (struct NodeIntrusiveLinks){ NULL, NULL }; \
node_header(Ty).list_destroyed = (struct NodeIntrusiveLinks){ NULL, NULL }; \
node_header(Ty).list_free = (struct NodeIntrusiveLinks){ NULL, NULL }; \
node_header(Ty).construct = construct_f; \
node_header(Ty).process = process_f; \
node_header(Ty).destruct = destruct_f;

#define node_meta_defines(Ty)\
NodeHeader node_header(Ty);

typedef void AnyNode;

extern void *node_new_from_header(NodeHeader *header);

// This function should not be called by user-facing code generally.
// 
// Collects all the nodes from the destroyed list into the freed list, so
// they can be re-used.
extern void node_header_collect_destroyed_list(NodeHeader *header);

#define node_new(Ty) ((Ty*)node_new_from_header(&node_header(Ty)))
#define new(Ty) node_new(Ty)

extern void node_destroy(AnyNode *node);

/* The raw Node type. This type is responsible for a lot of stuff, including
 * both the basic scene tree AND the transform heirarchy (as they are extremely
 * interconnected in this game engine.
 */

#define FieldList_Node \
struct NodeIntrusiveLinks alloc_info; \
NodeHeader *header; \
struct Node *parent; \
list_of(struct Node*) children; \
RawTransform raw_tform;

node_from_field_list(Node)



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

void ltranslate(AnyNode *node, vec2 offset);
void gtranslate(AnyNode *node, vec2 offset);

vec2 get_basis_x(AnyNode *node);
vec2 get_basis_y(AnyNode *node);

vec2 local_to_global(AnyNode *node, vec2 local);
vec2 global_to_local(AnyNode *node, vec2 global);

const RawTransform *node_get_parent_transform(Node *node);
bool node_update_transform(Node *node);

void reparent(AnyNode *child, AnyNode *new_parent);