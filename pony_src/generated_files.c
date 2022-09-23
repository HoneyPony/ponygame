#include "pony.h"

#include <stdio.h>

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