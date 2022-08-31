#include <stdlib.h>

#include "pony_render.h"

#include "pony_opengl.h"
#include "pony_log.h"

typedef struct {
	vec2 top_left;
	vec2 top_right;
	vec2 bottom_left;
	vec2 bottom_right;
	GLuint texture;
	vec2 uv_bottom_left;
	vec2 uv_top_right;
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

static list_of(float) vertex_list = NULL;
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

void render_init_lists() {
	ls_init(opaque_list);
	ls_init(transparent_list);

	ls_init(vertex_list);
	ls_init(state_spec_list);
}

void render_tex_on_node(AnyNode *node, TexHandle *tex, vec2 tex_pivot, bool snap) {
	vec2 center = get_gpos(node);
	// TODO: Transform by camera... or do we just want to snap the camera...?
	
	if(snap) {
		center = round(center);
	}

	vec2 basis_x = get_basis_x(node);
	vec2 basis_y = get_basis_y(node);

	vec2 left  = mul(basis_x,                 -tex_pivot.x);
	vec2 right = mul(basis_x, tex->px_size.x - tex_pivot.x);
	vec2 down  = mul(basis_y,                 -tex_pivot.y);
	vec2 up    = mul(basis_y, tex->px_size.y - tex_pivot.y);

	RenderCommand cmd;

	cmd.z_index = 0; // TODO: Z index
	cmd.type = CMD_SPRITE;
	cmd.opaque = 1; // TODO: Support transparency computation

	cmd.sprite.top_left =     add(center, add(up,   left));
	cmd.sprite.top_right =    add(center, add(up,   right));
	cmd.sprite.bottom_left =  add(center, add(down, left));
	cmd.sprite.bottom_right = add(center, add(down, right));

	cmd.sprite.texture = tex->texture;

	cmd.sprite.uv_bottom_left = tex->uv_bottom_left;
	cmd.sprite.uv_top_right = tex->uv_top_right;

	PUSH_CMD_VAR(cmd);
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
			float z = cmd->z_index;

			float uv_b = cmd->sprite.uv_bottom_left.y;
			float uv_l = cmd->sprite.uv_bottom_left.x;
			float uv_t = cmd->sprite.uv_top_right.y;
			float uv_r = cmd->sprite.uv_top_right.x;


			// NOTE: If a backslash is missing from these macros, very bad
			// things happen, and it is very hard to tell what is wrong.
			//
			// Perhaps the macros should be moved somewhere else, where a 
			// missing backslash will result in a compiler error.
			#define PUSH_BL \
				ls_push_var(vertex_list, cmd->sprite.bottom_left.x);\
				ls_push_var(vertex_list, cmd->sprite.bottom_left.y);\
				ls_push_var(vertex_list, z);\
				ls_push_var(vertex_list, uv_l);\
				ls_push_var(vertex_list, uv_b);

			#define PUSH_BR \
				ls_push_var(vertex_list, cmd->sprite.bottom_right.x);\
				ls_push_var(vertex_list, cmd->sprite.bottom_right.y);\
				ls_push_var(vertex_list, z);\
				ls_push_var(vertex_list, uv_r);\
				ls_push_var(vertex_list, uv_b);

			#define PUSH_TR \
				ls_push_var(vertex_list, cmd->sprite.top_right.x);\
				ls_push_var(vertex_list, cmd->sprite.top_right.y);\
				ls_push_var(vertex_list, z);\
				ls_push_var(vertex_list, uv_r);\
				ls_push_var(vertex_list, uv_t);

			#define PUSH_TL \
				ls_push_var(vertex_list, cmd->sprite.top_left.x);\
				ls_push_var(vertex_list, cmd->sprite.top_left.y);\
				ls_push_var(vertex_list, z);\
				ls_push_var(vertex_list, uv_l);\
				ls_push_var(vertex_list, uv_t);

			// First triangle
			PUSH_BL
			PUSH_BR
			PUSH_TR

			// Second triangle
			PUSH_BL
			PUSH_TR
			PUSH_TL

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

// Because we're using WebGL 1.0 (IIRC), we need to manually specify attribute
// arrays each time... vao's are not supported.
void attrib_array_sprites() {
	// First attribute: Position: x, y, and z.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); 

	// Second attribute: UV's.
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void render_state_spec_list() {
	attrib_array_sprites();

	for(uint32_t i = 0; i < ls_length(state_spec_list); ++i) {
		RenderStateSpec *spec = &state_spec_list[i];

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, spec->texture);

		glDrawArrays(GL_TRIANGLES, spec->draw_start, spec->draw_count);
	}
}

// Renders all the render lists.
void render_lists(GLuint vbo) {
	// First step: sort render lists.
	render_sort_render_lists();

	// Second step: Build vertex list and state specification list.
	render_build_state_spec();

	// Third step: Upload vertex data to GPU.
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER,
		ls_length(vertex_list) * sizeof(*vertex_list),
		vertex_list,
		GL_DYNAMIC_DRAW);

	// Fourth step: Render state spec list.
	render_state_spec_list();

	// Fifth step: Clear lists.
	ls_clear(opaque_list);
	ls_clear(transparent_list);

	ls_clear(vertex_list);
	ls_clear(state_spec_list);
}