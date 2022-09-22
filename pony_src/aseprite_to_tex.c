#include "cjson/cJSON.h"

#include <stdio.h>
#include <stdlib.h>

#include "pony.h"

void process_aseprite(TexBuildInfo *tb) {
	FILE *f = NULL;

	char aseprite_cmd[2048] = {0};
	// TODO: Support configurable Aseprite path
	snprintf(aseprite_cmd, 2048, "\"C:\\Program Files\\Aseprite\\Aseprite.exe\" -b %s --sheet %s --format json-array --list-tags", tb->aseprite_source, tb->image_source);

    f = popen(aseprite_cmd, "rb");

	// Need to accumulate data over time as we are using popen, not a normal file
	// that can be accessed with random access
	str data = str_blank();
	for(;;) {
		int next = fgetc(f);
		if(next == EOF) break;

		str_push(data, (char)next);
	}

    pclose(f);
	//puts(data);

	FILE *out = fopen(tb->tex_path, "w");

	fputs("# This .tex file contains animation data for the associated texture.\n", out);
	fprintf(out, "@load %s\n", tb->image_source);
	fprintf(out, "@from-aseprite %s\n\n", tb->aseprite_source);

	cJSON *result = cJSON_Parse(data);
	cJSON *meta = cJSON_GetObjectItem(result, "meta");
	cJSON *tags = cJSON_GetObjectItem(meta, "frameTags");

	cJSON *frames = cJSON_GetObjectItem(result, "frames");

	if(cJSON_IsArray(tags)) {
		cJSON *tag = NULL;
		cJSON_ArrayForEach(tag, tags) {
			cJSON *name = cJSON_GetObjectItem(tag, "name");
			cJSON *from = cJSON_GetObjectItem(tag, "from");
			cJSON *to = cJSON_GetObjectItem(tag, "to");

			if(cJSON_IsString(name) && (name->valuestring) && cJSON_IsNumber(from) && cJSON_IsNumber(to)) {
				fprintf(out, "@anim %s\n", name->valuestring);

				for(int i = from->valueint; i <= to->valueint; ++i) {
					cJSON *anim_frame = cJSON_GetArrayItem(frames, i);

					cJSON *frame = cJSON_GetObjectItem(anim_frame, "frame");
					cJSON *duration = cJSON_GetObjectItem(anim_frame, "duration");

					cJSON *x = cJSON_GetObjectItem(frame, "x");
					cJSON *y = cJSON_GetObjectItem(frame, "y");
					cJSON *w = cJSON_GetObjectItem(frame, "w");
					cJSON *h = cJSON_GetObjectItem(frame, "h");

					if(cJSON_IsNumber(duration) &&
						cJSON_IsNumber(x) &&
						cJSON_IsNumber(y) &&
						cJSON_IsNumber(w) &&
						cJSON_IsNumber(h) 
					) {
						fprintf(out, "\t@frame %dms %d %d %d %d\n", duration->valueint, x->valueint, y->valueint, w->valueint, h->valueint);
					}
				}

				fputs("\n", out);
			}
		}
	}

	fclose(out);

	cJSON_Delete(result);

    str_free(data);
}

void process_aseprite_from_tex(const char *path) {
	TexBuildInfo tb = load_tex_build_info(path);
	process_aseprite(&tb);
}