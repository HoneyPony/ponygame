#include <stdlib.h>
#include <string.h>

#include "render.h"
#include "pony_unsafe_transforms.h"

// Renderer lists
static list_of(TexRenderer) tex_renderer_list;

typedef struct {
	GLuint texture;
	uint32_t index;
} SpriteRenderCommand;

typedef struct {
	union {
		SpriteRenderCommand sprite;
	};
	int32_t z_index;
	int8_t type;
	int8_t opaque;
} RenderCommand;

typedef struct {
	// Indices for glDrawArrays()
	size_t draw_start;
	size_t draw_count;

	// Texture handle
	GLuint texture;

	// Command type
	int8_t type;
} RenderStateSpec;

#define CMD_SPRITE 0

static list_of(RenderCommand) opaque_list = NULL;
static list_of(RenderCommand) transparent_list = NULL;

static struct {
	float *list;
	size_t alloc;
	size_t length;
} vertex_list;

static struct {
	uint32_t *list;
	size_t alloc;
	size_t length;
} element_list;

static list_of(RenderStateSpec) state_spec_list = NULL;

#define PUSH_CMD_VAR(cmd)\
do {\
if(cmd.opaque) {\
	ls_push_var(opaque_list, cmd);\
}\
else {\
	ls_push_var(transparent_list, cmd);\
}\
} while(0)

void render_init_objects() {
	ls_init(tex_renderer_list);

	ls_init(opaque_list);
	ls_init(transparent_list);

	vertex_list.list = pony_malloc(sizeof(float) * 1024);
	vertex_list.alloc = 1024;
	vertex_list.length = 0;

	element_list.list = pony_malloc(sizeof(uint32_t) * 1024);
	element_list.alloc = 1024;
	element_list.length = 0;

	ls_init(state_spec_list);
}

void render_tex_on_node(TexRenderer tr) {
	ls_push(tex_renderer_list, tr);
}

static uint32_t push_vertices(float vertices[20]) {
	if(vertex_list.length + 20 > vertex_list.alloc) {
		vertex_list.alloc *= 2;
		vertex_list.list = pony_realloc(vertex_list.list, vertex_list.alloc * sizeof(float));
	}

	memcpy(vertex_list.list + vertex_list.length, vertices, sizeof(float) * 20);
	uint32_t result = (uint32_t)(vertex_list.length / 5);

	vertex_list.length += 20;
	return result;
}

void render_tex_renderer(TexRenderer tr) {
	vec2 center = get_gpos(tr.node);
	// TODO: Transform by camera... or do we just want to snap the camera...?
	
	if(tr.snap) {
		center = round(center);
	}

	vec2 basis_x = get_basis_x_fast(tr.node);
	vec2 basis_y = get_basis_y_fast(tr.node);

	vec2 left  = mul(basis_x,                     -tr.tex_pivot.x);
	vec2 right = mul(basis_x, tr.tex->px_size.x - tr.tex_pivot.x);
	vec2 down  = mul(basis_y,                     -tr.tex_pivot.y);
	vec2 up    = mul(basis_y, tr.tex->px_size.y - tr.tex_pivot.y);

	RenderCommand cmd;

	cmd.z_index = 0; // TODO: Z index
	cmd.type = CMD_SPRITE;
	cmd.opaque = 1; // TODO: Support transparency computation

	union {
		float array[20];
		struct {
			vec2 top_left;
			float z1;
			vec2 top_left_uv;
			vec2 top_right;
			float z2;
			vec2 top_right_uv;
			vec2 bottom_right;
			float z3;
			vec2 bottom_right_uv;
			vec2 bottom_left;
			float z4;
			vec2 bottom_left_uv;
		};
	} vertex_data;

	vertex_data.top_left =     add(center, add(up,   left));
	vertex_data.top_right =    add(center, add(up,   right));
	vertex_data.bottom_left =  add(center, add(down, left));
	vertex_data.bottom_right = add(center, add(down, right));

	vertex_data.bottom_left_uv = tr.tex->bottom_left_uv;
	vertex_data.bottom_right_uv = tr.tex->bottom_right_uv;
	vertex_data.top_left_uv = tr.tex->top_left_uv;
	vertex_data.top_right_uv = tr.tex->top_right_uv;

	// TODO: z-index
	float z_index = 0;

	vertex_data.z1 = z_index;
	vertex_data.z2 = z_index;
	vertex_data.z3 = z_index;
	vertex_data.z4 = z_index;

	cmd.sprite.texture = tr.tex->texture;
	cmd.sprite.index = push_vertices(vertex_data.array);

	PUSH_CMD_VAR(cmd);
}

void render_process_commands() {
	for(uint32_t i = 0; i < ls_length(tex_renderer_list); ++i) {
		render_tex_renderer(tex_renderer_list[i]);
	}
}

int compare_sprite_command_opaque(const RenderCommand *a, const RenderCommand *b) {
	int tex_dif = b->sprite.texture - a->sprite.texture;
	if(tex_dif != 0) return tex_dif;

	// Same tex = sort by Z
	int z_dif = b->z_index - a->z_index;
	return z_dif;
}

int compare_opaque(const void *av, const void *bv) {
	const RenderCommand *a = av;
	const RenderCommand *b = bv;

	// For opaque commands, we prefer to do the following:
	// 1. Chunk together operations with no state changes. This means:
	//    - Same kind of command, e.g. Sprite vs non-Sprite
	//    - Same shader, e.g. for sprites
	//    - Same texture handle
	// 2. After chunking operations, we want to sort closer things first.
	//    - This minimizes overdraw.
	if(a->type != b->type) {
		return (int)b->type - (int)a->type;
	}

	// At this point, the type must be equal. We can therefore split this into
	// different sorting comparators for different sub-types.
	switch(a->type) {
		case CMD_SPRITE:
			return compare_sprite_command_opaque(a, b);
		default:
			return 0;
	}
}

int compare_sprite_command_transparent(const RenderCommand *a, const RenderCommand *b) {
	// Only relevant state change is tex dif...
	int tex_dif = b->sprite.texture - a->sprite.texture;
	return tex_dif;
}

int compare_transparent(const void *av, const void *bv) {
	const RenderCommand *a = av;
	const RenderCommand *b = bv;

	// For transparent objects (in this case, transparent means any *partial*
	// transparency), we must always sort back-to-front. This is to get correct
	// drawing results.
	//
	// Note that an "opaque" object is allowed to have wholly-transparent pixels.
	// Those can be handled with alpha-scissor.
	int z_dif = b->z_index - a->z_index;
	if(z_dif != 0) return z_dif;

	// If the Z is the same, then we can sort by state changes.
	int type_dif = b->z_index - a->z_index;
	if(type_dif != 0) return type_dif;

	switch(a->type) {
		case CMD_SPRITE:
			return compare_sprite_command_transparent(a, b);
		default:
			return 0;
	}
}

void render_sort_render_lists() {
	qsort(
		opaque_list,
		ls_length(opaque_list),
		sizeof(*opaque_list),
		compare_opaque
	);

	qsort(
		transparent_list,
		ls_length(transparent_list),
		sizeof(*transparent_list),
		compare_transparent
	);
}

void push_elements(uint32_t elements[6]) {
	if(element_list.length + 6 > element_list.alloc) {
		element_list.alloc *= 2;
		element_list.list = pony_realloc(element_list.list, element_list.alloc * sizeof(uint32_t));
	}

	memcpy(element_list.list + element_list.length, elements, 6 * sizeof(uint32_t));
	element_list.length += 6;
}

void render_build_state_spec_for_list(list_of(RenderCommand) list) {
	RenderStateSpec next;
	next.draw_start = 0;
	next.draw_count = 0;
	for(uint32_t i = 0; i < ls_length(list); ++i) {
		RenderCommand *cmd = &list[i];
		if(i == 0)
			goto new_state_spec_no_push;
		
		if(cmd->type != next.type)
			goto new_state_spec;
		if(cmd->type == CMD_SPRITE) {
			if(cmd->sprite.texture != next.texture) goto new_state_spec;
		}

		// If there is no state spec change, we can go straight to pushing
		// the vertices.

		goto push_vertices;

new_state_spec:
		// New state spec means we need to push the old state spec.
		ls_push_var(state_spec_list, next);

new_state_spec_no_push:
		// Initialize the next value with the relevant data.
		next.type = cmd->type;
		next.draw_start += next.draw_count;
		next.draw_count = 0;
		if(cmd->type == CMD_SPRITE) {
			next.texture = cmd->sprite.texture;
		}

push_vertices:
		// For every single object we need to push vertex data.
		
		if(cmd->type == CMD_SPRITE) {
			uint32_t indices[6];
			uint32_t base = cmd->sprite.index;

			// First triangle
			indices[0] = 0 + base;
			indices[1] = 1 + base;
			indices[2] = 2 + base;

			// Second triangle
			indices[3] = 0 + base;
			indices[4] = 2 + base;
			indices[5] = 3 + base;

			push_elements(indices);

			// 6 vertices per quad
			next.draw_count += 6;
		}
	}

	// If there is anything left that needs pushing, push it to the list.
	if(next.draw_count > 0) {
		ls_push_var(state_spec_list, next);
	}
}

void render_build_state_spec() {
	// First: Render opaque objects, then render transparent objects.
	render_build_state_spec_for_list(opaque_list);
	render_build_state_spec_for_list(transparent_list);
}

void render_state_spec_list() {
	render_bind_sprite();

	for(uint32_t i = 0; i < ls_length(state_spec_list); ++i) {
		RenderStateSpec *spec = &state_spec_list[i];

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, spec->texture);

		glDrawElements(GL_TRIANGLES, spec->draw_count, GL_UNSIGNED_INT, 
			(void*)(spec->draw_start * sizeof(uint32_t)));
	}
}

// Renders all the render lists.
void render_objects() {
	// First step: process all pushed commands.
	render_process_commands();

	// Second step: sort render lists.
	render_sort_render_lists();

	// Third step: Build vertex list and state specification list.
	render_build_state_spec();

	// Third step: Upload vertex data to GPU.
	glBindBuffer(GL_ARRAY_BUFFER, ctx.sprite_render.vbo);
	glBufferData(GL_ARRAY_BUFFER,
		vertex_list.length * sizeof(float),
		vertex_list.list,
		GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx.sprite_render.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		element_list.length * sizeof(uint32_t),
		element_list.list,
		GL_DYNAMIC_DRAW);

	// Fourth step: Render state spec list.
	render_state_spec_list();

	// Fifth step: Clear lists.
	ls_clear(tex_renderer_list);

	ls_clear(opaque_list);
	ls_clear(transparent_list);

	//ls_clear(vertex_list);
	ls_clear(state_spec_list);

	vertex_list.length = 0;
	element_list.length = 0;
}