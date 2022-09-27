#include <stdio.h>
#include <stdlib.h>

#include "pony.h"

#define READ_UKNOWN        0
#define READ_STRUCT_FIELDS 1
#define READ_HEADER_LINES  2
#define READ_TREE_LINES    3

void push_tree_entry(list_of(PonyFileTreeEntry) *list, PonyFileTreeEntry tree, bool *tree_has_self_entry, const char *path) {
	if(str_eq_cstr(tree.name, "self")) {
		if(*tree_has_self_entry) {
			printf("error: in %s:\n\tduplicated self entry\n", path);
			exit(-1);
		}
		*tree_has_self_entry = true;
	}
	else if(!tree.parent) {
		printf("error: in %s:\n\tnon-self entry has no parent: %s\n", path, tree.name);
		exit(-1);
	}

	ls_push(*list, tree);
}

PonyFileInfo load_pony_file(const char *path) {
	FILE *in = fopen(path, "r");

	int read_mode = READ_UKNOWN;

	PonyFileInfo result = {0};
	// Warning! This might change if we ever change the file path algorithm.
	result.file_path = str_from(path + 2);
	ls_init(result.header_lines);
	ls_init(result.struct_fields);
	ls_init(result.tree_entries);

	PonyFileTreeEntry tree = {0};
	ls_init(tree.initializers);

	bool tree_has_self_entry = false;

	while(!feof(in)) {
		char line[1024];
		char var1[1024];
		char var2[1024];
		fgets(line, 1024, in);

		if(line[0] == '@') {
			if(cstr_has_prefix(line, "@type")) {
				if(sscanf(line, "@type %s : %s", var1, var2) != 2) {
					printf("error: in %s:\n\tcould not read type name and base class name\n", path);
					exit(-1);
				}
				read_mode = READ_STRUCT_FIELDS;
				result.type_name = str_from(var1);
				str_wtrim_back(result.type_name);
				result.type_base_class = str_from(var2);
				str_wtrim_back(result.type_base_class);
			}
			else if(cstr_has_prefix(line, "@header")) {
				read_mode = READ_HEADER_LINES;
			}
			else if(cstr_has_prefix(line, "@tree")) {
				read_mode = READ_TREE_LINES;
			}
			else if(cstr_has_prefix(line, "@tick")) {
				result.has_tick = true;
			}
			else if(cstr_has_prefix(line, "@construct")) {
				result.has_construct = true;
			}
			else if(cstr_has_prefix(line, "@destruct")) {
				result.has_destruct= true;
			}
			else {
				printf("error: in %s:\n\tunknown @ line: %s\n", path, line);
				exit(-1);
			}
		}

		if(read_mode == READ_STRUCT_FIELDS) {
			if(line[0] == '\t') {
				str field = str_from(line + 1);
				str_wtrim_back(field);
				ls_push(result.struct_fields, field);
			}
		}
		if(read_mode == READ_HEADER_LINES) {
			if(line[0] == '\t') {
				str header_line = str_from(line + 1);
				str_wtrim_back(header_line);
				ls_push(result.header_lines, header_line);
			}
		}
		if(read_mode == READ_TREE_LINES) {
			if(cstr_has_prefix(line, "\t\t")) {
				if(sscanf(line, " parent = %s\n", var1) == 1) {
					tree.parent = str_from(var1);
				}
				else {
					str initializer = str_blank();
					char *read = line;
					bool trim = true;

					// Use a for loop so that the pointer will be advanced every
					// time we use 'continue'
					for(; *read; ++read) {
						// Trim any whitespace at the front.
						if(is_whitespace(*read)) {
							if(trim) continue;
						}
						else {
							trim = false;
						}

						// Always skip newlines.
						if(*read == '\n') continue;

						// Substitute $ for the object name
						if(*read == '$') {
							if(!tree.name) {
								printf("error: in %s:\n\ttrying to use $ but there is no name: %s\n", path, line);
								exit(-1);
							}
							str_append_str(initializer, tree.name);
							continue;
						}

						// If it fits no other case, just append the character
						str_push(initializer, *read);
					}

					if(str_rfind(initializer, ';') == -1) {
						printf("warning: in %s: added semicolon to '%s'\n", path, initializer);
						str_push(initializer, ';');
					}

					ls_push(tree.initializers, initializer);
				}
			}
			else if(line[0] == '\t') {
				if(sscanf(line, " %s : %s", var1, var2) == 2) {
					if(tree.name && tree.type) {
						push_tree_entry(&result.tree_entries, tree, &tree_has_self_entry, path);

						tree = (PonyFileTreeEntry){0};
						ls_init(tree.initializers);
					}

					tree.name = str_from(var1);
					tree.type = str_from(var2);
				}
				else {
					printf("error: in %s:\n\tcannot parse tree line: %s\n", path, line);
					exit(-1);
				}
			}
		}
	}

	if(tree.name && tree.type) {
		push_tree_entry(&result.tree_entries, tree, &tree_has_self_entry, path);
	}
	result.has_tree = !ls_empty(result.tree_entries);

	fclose(in);

	if(result.has_construct && !result.construct_func) {
		if(!result.type_name) {
			printf("error: in %s:\n\t@construct specified but no type name\n", path);
		}
		result.construct_func = str_fromf("construct_%s", result.type_name);
	}
	if(result.has_destruct && !result.destruct_func) {
		if(!result.type_name) {
			printf("error: in %s:\n\t@destruct specified but no type name\n", path);
		}
		result.destruct_func = str_fromf("destruct_%s", result.type_name);
	}
	if(result.has_tick && !result.tick_func) {
		if(!result.type_name) {
			printf("error: in %s:\n\t@tick specified but no type name\n", path);
		}
		result.tick_func = str_fromf("tick_%s", result.type_name);
	}

	if(result.has_tree && !tree_has_self_entry) {
		printf("error: in %s:\n\ttree specified but no 'self' node\n", path);
		exit(-1);
	}

	return result;
}