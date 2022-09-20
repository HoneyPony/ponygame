#include <stdio.h>

#include "pony.h"

#include "stb_image.h"

unsigned char *load_image_data(const char *arg, int *x, int *y) {
	return stbi_load(arg, x, y, NULL, 4);
}

TexBuildInfo load_tex_build_info(const char *path) {
	FILE *in = fopen(path, "r");

	TexBuildInfo info = { NULL, NULL };

	char line[1024];
	char arg[1024];
	while(!feof(in)) {
		line[0] = 0;
		arg[0] = 0;
		fgets(line, 1024, in);
		if(sscanf(line, "@load %s", arg) == 1) {
			info.image_source = str_from(arg);
		}
		if(sscanf(line, "@from-aseprite %s", arg) == 1) {
			info.aseprite_source = str_from(arg);
		}
	}

	fclose(in);

	return info;
}

void load_tex_file(const char *path, list_of(TexFileInfo) *tex_out, list_of(AnimInfo) *anim_out) {
	FILE *in = fopen(path, "r");

	unsigned char *image_data = NULL;
	int x_stride = 0;
	int y_stride = 0;

	AnimInfo next;
	next.name = NULL;
	next.frame_count = 0;

	str image_source = NULL;
	str aseprite_source = NULL;

	char line[1024];
	char arg[1024];
	while(!feof(in)) {
		fgets(line, 1024, in);
		if(sscanf(line, "@load %s", arg) == 1) {
			image_data = load_image_data(arg, &x_stride, &y_stride);
			image_source = str_from(arg);
		}
		if(sscanf(line, "@from-aseprite %s", arg) == 1) {
			aseprite_source = str_from(arg);
		}
		else if(sscanf(line, "@anim %s", arg) == 1) {
			if(next.frame_count) {
				ls_push(*anim_out, next);
				next.frame_count = 0;
				next.name = NULL;
			}
			next.name = str_from(arg);
		}
		else {
			TexFileInfo info;
			if(sscanf(line, "\t@frame %dms %llu %llu %llu %llu", &info.anim_length, &info.x, &info.y, &info.width, &info.height) == 5) {
				// Image specific animation data
				info.anim_name = next.name;
				info.anim_frame = next.frame_count;

				// RGB image data
				info.data_stride = x_stride;
				info.data = image_data;

				// Metadata used for build files
				info.image_source = image_source;
				info.aseprite_source = aseprite_source;

				// Keep track of frame count for current animation info
				next.frame_count += 1;

				ls_push(*tex_out, info);
			}
		}
	}

	// Push last anim entry if it isn't already pushed
	if(next.frame_count) {
		ls_push(*anim_out, next);
	}

	fclose(in);
}

void test_tex(const char *path) {
	list_of(TexFileInfo) tex_list;
	list_of(AnimInfo) anim_list;

	ls_init(tex_list);
	ls_init(anim_list);

	load_tex_file(path, &tex_list, &anim_list);
	foreach(tex, tex_list, {
		printf("got texture at %d,%d in %s, frame %d\n", tex.x, tex.y, tex.anim_name, tex.anim_frame);
	})

	foreach(anim, anim_list, {
		printf("anim %s has %d frames\n", anim.name, anim.frame_count);
	})
}