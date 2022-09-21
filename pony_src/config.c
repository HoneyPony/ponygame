#include <stdio.h>

#include "pony.h"

Config load_config() {
	Config result;
	ls_init(result.include_paths);
    ls_init(result.lib_paths);
    result.lib_file = NULL;

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
	}
done_reading:
	str_free(var_name);
	str_free(var_val);

	return result;
}