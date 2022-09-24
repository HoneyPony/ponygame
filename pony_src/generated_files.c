#include <stdio.h>
#include <stdlib.h>

#include "pony.h"



static const char *res_debug_c = 
"#include \"pony_fs.h\"\n"
"\n"
"bool pony_resources_are_packed = false;\n"
"\n"
"FSPackedMem fs_find_packed_resource(const char *res_path) {\n"
"\treturn (FSPackedMem){ 0 };\n"
"}\n";

void generate_file_res_debug_c() {
	FILE *out = fopen(".ponygame/res_debug.c", "w");

	fputs(res_debug_c, out);

	fclose(out);
}

void pack_res(const char *fname, FILE *out, FILE *list, size_t *index) {
	FILE *in = fopen(fname, "rb");

	printf("packing file %s...\n", fname);

	size_t start = *index;
	for(;;) {
		int next = fgetc(in);
		if(next == EOF) break;

		fprintf(out, "%d,", next);
		(*index)++;
	}
	size_t size = *index - start;
	fprintf(list, "if(!strcmp(res_path,\"%s\")) { result.ptr = data + %llu; result.length = %llu; return result; }", fname, start, size);

	fclose(in);
}

void generate_file_res_release_c(ProjectFiles *pf) {
	FILE *out = fopen(".ponygame/res_release.c", "w");
	FILE *list = fopen(".ponygame/res_release_tree.h", "w");

	fputs("#include <string.h>\n#include \"pony_fs.h\"\n\n", out);
	fputs("bool pony_resources_are_packed = true;\n\n", out);

	size_t index = 0;
	
	fputs("static uint8_t data[] = {", out);

	for(int i = 0; i < pf->spritesheet_count; ++i) {
		char tmp[256] = { 0 };
		snprintf(tmp, 256, "pngcrush -ow .ponygame/spritesheet%04d.png", i);
		system(tmp);

		snprintf(tmp, 256, ".ponygame/spritesheet%04d.png", i);
		pack_res(tmp, out, list, &index);
	}

	printf("packing %d sounds\n", ls_length(pf->path_list.snd_infos));
	foreach(snd_info, pf->path_list.snd_infos, {
		if(!snd_info.sound_source) {
			printf("pack file warning: bad .snd file '%s'\n", snd_info.snd_path);
		}

		pack_res(snd_info.sound_source, out, list, &index);
	})

	fputs("};\n", out);

	fputs("FSPackedMem fs_find_packed_resource(const char *res_path) {\n", out);
	fputs("FSPackedMem result = { 0 };\n", out);
	fputs("#include \"res_release_tree.h\"\n", out);
	fputs("return result;\n}\n", out);

	fclose(list);
	fclose(out);
}