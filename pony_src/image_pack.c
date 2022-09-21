#include "stb_rect_pack.h"
#include "stb_image_write.h"

#include <stdio.h>
#include <string.h>
#include "pony.h"

const int tex_width = 1024;

void write_to_image(unsigned char *img, int x, int y, TexFileInfo *tf) {
	for(size_t j = 0; j < tf->height; ++j) {
		for(size_t i = 0; i < tf->width; ++i) {
			int ox = x + i;
			int oy = y + j;

			int out_index = ((oy * tex_width) + ox) * 4;
			
			int in_index = (((tf->y + j) * tf->data_stride) + (tf->x + i)) * 4;

			memcpy(img + out_index, tf->data + in_index, 4 * sizeof(*img));
		}
	}
}

void build_image(int number, list_of(struct stbrp_rect) *rects, list_of(TexFileInfo*) tex_list) {
	unsigned char *img_data = malloc(4 * sizeof(*img_data) * tex_width * tex_width);

	list_of(struct stbrp_rect) new_list;
	ls_init(new_list);

	int packed_total = 0;

	foreach(rect, (*rects), {
		if(rect.was_packed) {
			TexFileInfo *tf = tex_list[rect.id];
			write_to_image(img_data, rect.x, rect.y, tf);

            // Update data
            tf->spritesheet_index = number;
            tf->coords.sx = (uint16_t)rect.x;
            tf->coords.sy = (uint16_t)rect.y;
            tf->coords.ex = (uint16_t)(rect.x + rect.w);
            tf->coords.ey = (uint16_t)(rect.y + rect.h);
			packed_total += 1;
			//printf("[%s] generated coords: %d, %d, %d, %d\n", tf->anim_name, tf->coords.sx, tf->coords.sy, tf->coords.ex, tf->coords.ey);
		}
		else {
			ls_push(new_list, rect);
		}
	})

	printf("Packed %d images\n", packed_total);

	char file_path[1024] = { 0 };
	snprintf(file_path, 1024, ".ponygame/spritesheet%04d.png", number);

	stbi_write_png(file_path, tex_width, tex_width, 4, img_data, (tex_width * 4 * sizeof(*img_data)));

	free(img_data);

	ls_free(*rects);
	*rects = new_list;
}

void pack_images(ProjectFiles *pf) {
	struct stbrp_context ctx;
	struct stbrp_node *nodes = pony_malloc(sizeof(*nodes) * tex_width);

	list_of(struct stbrp_rect) rects;
	ls_init(rects);

	int index = 0;
	foreach(frame, pf->tex_list, {
		struct stbrp_rect rect;
		rect.w = frame->width;
		rect.h = frame->height;

		rect.id = index++;

		ls_push(rects, rect);
	})

	int image_index = 0;

	for(;;) {
		stbrp_init_target(&ctx, tex_width, tex_width, nodes, tex_width);
		int all_packed = stbrp_pack_rects(&ctx, rects, ls_length(rects));

		build_image(image_index, &rects, pf->tex_list);
		image_index += 1;

		if(all_packed || ls_empty(rects)) {
			break;
		}
	}

    pf->spritesheet_count = image_index;

    // Touch the lock file, as that's what the build system expects.
    FILE *lock = fopen(".ponygame/tex.lock", "w");
    fclose(lock);
}