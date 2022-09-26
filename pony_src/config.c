#include <stdio.h>

#include "pony.h"

Config load_config() {
	Config result;
	ls_init(result.include_paths);
    ls_init(result.lib_paths);
    result.lib_file = NULL;
	result.html_src_path = NULL;

	result.emcc = str_from("emcc");

	FILE *in = fopen(".ponygame/my.ponyconfig", "r");
	if(!in) return result;

	str var_name = str_blank();
	str var_val = str_blank();
	for(;;) {
next_line:

		str_clear(var_name);
		str_clear(var_val);

		for(;;) {
			int next = fgetc(in);
			if(next == '=') break;
			if(next == '\n') goto next_line;
			if(next == EOF) goto done_reading;

			str_push(var_name, (char)next);
		}

		for(;;) {
			int next = fgetc(in);
			if(next == '\n' || next == EOF) break;

			str_push(var_val, (char)next);
		}

		if(str_eq_cstr(var_name, "include_path")) {
			ls_push(result.include_paths, str_dupe(var_val));
		}

        if(str_eq_cstr(var_name, "lib_path")) {
            ls_push(result.lib_paths, str_dupe(var_val));
        }

        if(str_eq_cstr(var_name, "lib_file")) {
            result.lib_file = str_dupe(var_val);
        }

		if(str_eq_cstr(var_name, "html_src_path")) {
            result.html_src_path = str_dupe(var_val);
        }

		if(str_eq_cstr(var_name, "emcc")) {
			str_free(result.emcc);
			result.emcc = str_dupe(var_val);
		}
	}
done_reading:
	str_free(var_name);
	str_free(var_val);

	return result;
}

void generate_file_blank_ponyconfig() {
	FILE *out = fopen(".ponygame/my.ponyconfig", "w");
	fputs("include_path=C:/.../ponygame/src\n", out);
	fputs("lib_path=C:/.../ponygame\n", out);
	fputs("lib_file=/.../ponygame/libponygame.a\n", out);
	fputs("html_src_path=C$:/.../ponygame/web_src\n", out);
	fputs("emcc=C:/.../emsdk/upstream/emscripten/emcc.bat\n", out);
	fclose(out);
}