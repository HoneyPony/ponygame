#include <stdio.h>

#include "pony.h"

bool tex_file_is_static(DirTree *file) {
    return ls_empty(file->anim_infos) && (ls_length(file->tex_infos) == 1);
}

void build_tex_handle_statement(TexFileInfo *tf, FILE *out) {
    fprintf(out, "(TexHandle) {\n\t\tspritesheet%04d,\n", tf->spritesheet_index);

	//printf("[%s] generated coords: %d, %d, %d, %d\n", tf->anim_name, tf->coords.sx, tf->coords.sy, tf->coords.ex, tf->coords.ey);

    float sx, sy, ex, ey;

    sx = (float)tf->coords.sx / 1024.0;
    ex = (float)tf->coords.ex / 1024.0;

    // Vertical flip..?
    sy = (1024.0 - (float)tf->coords.sy) / 1024.0;
    ey = (1024.0 - (float)tf->coords.ey) / 1024.0;

    // Output UV's
    fprintf(out, "\t\tvxy(%.9g,%.9g),\n", sx, ey);
    fprintf(out, "\t\tvxy(%.9g,%.9g),\n", ex, ey);
    fprintf(out, "\t\tvxy(%.9g,%.9g),\n", sx, sy);
    fprintf(out, "\t\tvxy(%.9g,%.9g),\n", ex, sy);

    // Output texture size
    fprintf(out, "\t\tvxy(%.9g,%.9g)\n", (float)tf->width, (float)tf->height);

    fputs("\t};\n", out);
}


void build_tex_loader(ProjectFiles *pf, DirTree *tree, str prefix, FILE *out) {
    str sname = str_dupe(tree->name);
    str_replace(sname, '.', '_');

    if(tex_file_is_static(tree)) {
        fprintf(out, "\t%s%s = ", prefix, sname);
        build_tex_handle_statement(&tree->tex_infos[0], out);
    }
    else {
		foreach(anim, tree->anim_infos, {
			//fprintf(out, "\tlogf_info(\"loading %s\\n\");\n", sname);
			fprintf(out, "\t%s%s.%s = (AnimHandle) {\n", prefix, sname, anim.name);
			fprintf(out, "\t\t%d,\n", anim.frame_count);
			fprintf(out, "\t\tframe_memory + %d\n", pf->anim_frame_ptr);
			fprintf(out, "\t};\n");

			// Which frame to write data into.
			int frame = pf->anim_frame_ptr;

			for(int i = anim.index_first; i <= anim.index_last; ++i) {
				// These indices are for this list in particular.
				TexFileInfo *tf = &tree->tex_infos[i];

				fprintf(out, "\tframe_memory[%d].time_ms = %d;\n", frame, tf->anim_length);
				fprintf(out, "\tframe_memory[%d].texture = ", frame);
				build_tex_handle_statement(tf, out);

				frame += 1;
			}

			// Consume that many animation frames.
			pf->anim_frame_ptr += anim.frame_count;
		})
       /* indent(depth, out);
        fprintf(out, "struct %s {\n", sname);

        foreach(anim, tree->anim_infos, {
            indent(depth + 1, out);
            fprintf(out, "AnimHandle %s;\n", anim.name);
        })

        indent(depth, out);
        fprintf(out, "} %s;\n", sname);*/
    }

    str_free(sname);
}

void build_snd_loader(ProjectFiles *pf, DirTree *tree, str prefix, FILE *out) {
	str sname = str_dupe(tree->name);
    str_replace(sname, '.', '_');

	const char *sound_path = tree->snd_info.build_output;
	if(!sound_path) return;

	const char *function = tree->snd_info.is_music ? "fs_load_music" : "fs_load_sound";

	fprintf(out, "\t%s%s = %s(\"res://%s\");\n", prefix, sname, function, sound_path);

    str_free(sname);
}

str pony_tree_func_name(PonyFileInfo *pony) {
	str fname = str_from("tree_init_");
	str_append_str(fname, pony->file_path);
	// Use the file path for a (generally?) unique function name.
	str_replace(fname, '/', '_');
	str_replace(fname, '.', '_');
	return fname;
}

void build_pony_loader(ProjectFiles *pf, DirTree *tree, str prefix, FILE *out) {
	if(!tree->pony_info->has_tree) {
		return;
	}

	str sname = str_dupe(tree->name);
	if(cstr_has_suffix(sname, ".pony")) {
		// Pony files have the special privilege of simply not having a suffix
		// in the tree.
		// This is unique for now! Maybe later we can have a nicer interface for
		// all the files??
		str_rewind(sname, 5);
	}
    str_replace(sname, '.', '_');
	str fname = pony_tree_func_name(tree->pony_info);

	// TODO: Make the pony sname generation a function.

	// If the tree has no associated type, all we need to do is point the handle
	// at the associated function.
	// This is honestly kind of weird, as the other node-specific code generation
	// goes in its own file, but I guess it's fine for now...
	fprintf(out, "\t%s%s.instance = %s;\n", prefix, sname, fname);

	// If there is an associated type with the tree, then the header needs to
	// have its associated_tree pointed at the tree function as well.
	if(tree->pony_info->type_name) {
		fprintf(out, "\tnode_header(%s).associated_tree = %s;\n", tree->pony_info->type_name, fname);
	}

	str_free(fname);
    str_free(sname);
}

void build_resource_loader_tree(ProjectFiles *pf, DirTree *tree, str prefix, FILE *out) {
    if(tree->type == DIR_TREE_DIRECTORY) {
        str sname = str_dupe(tree->name);
        str_replace(sname, '.', '_');

        str total = str_dupe(prefix);
        str_append_str(total, sname);
        str_push(total, '.');

        foreach(dt, tree->children, {
            build_resource_loader_tree(pf, &dt, total, out);
        })

        str_free(total);
        str_free(sname);
    }

    if(tree->type == DIR_TREE_TEX) {
        build_tex_loader(pf, tree, prefix, out);
    }

	if(tree->type == DIR_TREE_SND) {
        build_snd_loader(pf, tree, prefix, out);
    }

	if(tree->type == DIR_TREE_PONY) {
        build_pony_loader(pf, tree, prefix, out);
    }
}

void resource_loader_allocate_frame_list(ProjectFiles *pf, FILE *out) {
	int frame_count = 0;
	foreach(anim, pf->anim_list, {
		frame_count += anim->frame_count;
	})

	fprintf(out, "static AnimFrame frame_memory[%d];\n\n", frame_count);
}

void build_pony_tree_instancers(ProjectFiles *pf, FILE *out) {
	foreach(pony, pf->pony_list, {
		if(!pony->has_tree) continue;
		str fname = pony_tree_func_name(pony);

		// TODO: Also make the TypenameTree if the pony file has a type associated
		// with it.

		fprintf(out, "static Node *%s() {\n", fname);
		// First step: allocate all the nodes in the tree.
		foreach(tree, pony->tree_entries, {
			// Need to use the unsafe node allocator, as the safe allocator will
			// try to allocate a tree, which ... would result in infinite
			// recursion
			fprintf(out, "\t%s *%s = node_new_from_header_unsafe(&node_header(%s));\n", tree.type, tree.name, tree.type);
		})

		// Second step: reparent nodes to their parents.
		foreach(tree, pony->tree_entries, {
			if(tree.parent) {
				fprintf(out, "\treparent(%s, %s);\n", tree.name, tree.parent);
			}
		})

		// Third step: run tree initializers. The user can control this by
		// rearranging nodes in their tree.
		foreach(tree, pony->tree_entries, {
			foreach(initializer, tree.initializers, {
				fprintf(out, "\t%s\n", initializer);
			})
		})

		// If this is a type, then there is a tree variable that needs to be
		// initialized.
		// We can't do it in the new() function, because that function just calls
		// this one -- so we have to initialize the tree here.
		if(pony->type_name) {
			foreach(tree, pony->tree_entries, {
				fprintf(out, "\tself->tree.%s = %s;\n", tree.name, tree.name);
			})
		}

		fputs("\treturn (Node*)self;\n", out);
		fputs("}\n\n", out);

		str_free(fname);
	})
}

void build_resource_loader(ProjectFiles *pf) {

    FILE *out = fopen(FILE_NAME_RES_LOADER, "w");

    fputs("#include \"my.ponygame.h\"\n\n", out);

	// Must declare the global res structure
	fputs("struct res res;\n", out);

	resource_loader_allocate_frame_list(pf, out);
	pf->anim_frame_ptr = 0;

	build_pony_tree_instancers(pf, out);

    fputs("void pony_load_resources() {\n", out);

    for(int i = 0; i < pf->spritesheet_count; ++i) {
        fprintf(out, "\tGLuint spritesheet%04d = gltex_load(\"res://.ponygame/spritesheet%04d.png\");\n", i, i);
    }
    fputs("\n", out);

    str prefix = str_from("res.");

    foreach(dt, pf->tree.children, {
        build_resource_loader_tree(pf, &dt, prefix, out);
    })

    str_free(prefix);

    fputs("}\n", out);

    fclose(out);
}

void indent(int depth, FILE *out) {
    for(int i = 0; i < depth; ++i) fputc('\t', out);
}

void build_tex_header(DirTree *tree, int depth, FILE *out) {
    str sname = str_dupe(tree->name);
    str_replace(sname, '.', '_');

    if(tex_file_is_static(tree)) {
        indent(depth, out);
        fprintf(out, "TexHandle %s;\n", sname);
    }
    else {
        indent(depth, out);
        fprintf(out, "struct %s {\n", sname);

        foreach(anim, tree->anim_infos, {
            indent(depth + 1, out);
            fprintf(out, "AnimHandle %s;\n", anim.name);
        })

        indent(depth, out);
        fprintf(out, "} %s;\n", sname);
    }

    str_free(sname);
}

void build_snd_header(DirTree *tree, int depth, FILE *out) {
	str sname = str_dupe(tree->name);
    str_replace(sname, '.', '_');

	indent(depth, out);
	const char *type = tree->snd_info.is_music ? "MusicHandle" : "SoundHandle";
	fprintf(out, "%s %s;\n", type, sname);

    str_free(sname);
}

void build_pony_header(DirTree *tree, int depth, FILE *out) {
	// The .pony file only counts as a resource if there is a tree to be instanced
	// there.
	if(!tree->pony_info->has_tree) {
		return;
	}

	str sname = str_dupe(tree->name);
	if(cstr_has_suffix(sname, ".pony")) {
		// Pony files have the special privilege of simply not having a suffix
		// in the tree.
		// This is unique for now! Maybe later we can have a nicer interface for
		// all the files??
		str_rewind(sname, 5);
	}
    str_replace(sname, '.', '_');

	// Pony files that count as resources are used to instance trees. This means
	// they can be called like this: res.something.instance()
	indent(depth, out);
	fprintf(out, "struct %s { TreeInstancer instance; } %s;\n", sname, sname);

    str_free(sname);
}

void build_resource_header_tree(ProjectFiles *pf, DirTree *tree, int depth, FILE *out) {
    if(tree->type == DIR_TREE_TEX) {
        build_tex_header(tree, depth, out);
    }

	if(tree->type == DIR_TREE_SND) {
        build_snd_header(tree, depth, out);
    }

	if(tree->type == DIR_TREE_PONY) {
		build_pony_header(tree, depth, out);
	}

    if(tree->type == DIR_TREE_DIRECTORY) {
        str sname = str_dupe(tree->name);
        str_replace(sname, '.', '_');

        indent(depth, out);
        fprintf(out, "struct %s {\n", sname);

        foreach(dt, tree->children, {
            build_resource_header_tree(pf, &dt, depth + 1, out);
        })

        indent(depth, out);
        fprintf(out, "} %s;\n", sname);

        str_free(sname);
    }
}

void build_resource_header(ProjectFiles *pf) {
    FILE *out = fopen(".ponygame/my.res.h.tmp", "w");
    fputs("#pragma once\n\n#include <ponygame.h>\n\n", out);

    fputs("struct res {\n", out);

    foreach(dt, pf->tree.children, {
        build_resource_header_tree(pf, &dt, 1, out);
    })

    fputs("};\n", out);

	fputs("\nextern struct res res;\n", out);

    fclose(out);
}

// Checks if the new resource header is different from the old one.
// If so, replaces it and returns true.
// Otherwise, does not replace it, and returns false.
bool check_and_replace_resource_header() {
	FILE *old = fopen(".ponygame/my.res.h", "r");
	FILE *new = fopen(".ponygame/my.res.h.tmp", "r");

	for(;;) {
		int a = fgetc(old);
		int b = fgetc(new);

		if(a != b) {
			fclose(old);
			fclose(new);

			remove(".ponygame/my.res.h");
			rename(".ponygame/my.res.h.tmp", ".ponygame/my.res.h");

			return true;
		}

		if(a == EOF) break; // Redundant? Shouldn't we know a==b at this point and return false?
		if(b == EOF) break;
	}

	fclose(old);
	fclose(new);

	return false;
}

void rebuild_resources(RebuildResourceArguments args) {
	BTime time = bt_start();

	puts("pony: scan & rebuild resources");

	
    ProjectFiles pf;

    pf.path_list = scan_for_files();
    pf.config = load_config();

    ls_init(pf.tex_list);
    ls_init(pf.anim_list);
	ls_init(pf.pony_list);

	// Load the directory tree...
	// TODO: Make this also generate the path list.
    get_dir_tree(&pf);

	puts("processing .pony files...");

	foreach(pony, pf.pony_list, {
		str dot_c = str_dupe(pony->file_path);
		str_append_cstr(dot_c, ".c");

		if(!already_exists(dot_c)) {
			generate_file_pony_c_from_pony(dot_c, pony);
			ls_push(pf.path_list.c_paths, dot_c);
		}
		else {
			str_free(dot_c);
		}
	})

	generate_file_my_ponygame_h(&pf);
	generate_file_pony_source_c(&pf);

	puts("building ninja files...");
	save_path_list(&pf.path_list);
	make_ninja_file(&pf.path_list, &pf.config, false, false);
	make_ninja_file(&pf.path_list, &pf.config, false, true);
	make_ninja_file(&pf.path_list, &pf.config, true, false);
	make_ninja_file(&pf.path_list, &pf.config, true, true);
    //foreach(path, list.tex_paths, {
    //    load_tex_file(path, &tex_list, &anim_list);
    //})

    
	if(args.no_sheet) {
		puts("--no-sheet: skipping spritesheet.");
	}
	else {
		puts("packing textures...");
		pack_images(&pf);
	}

    printf("building resource header...");
    build_resource_header(&pf);
	bool used = check_and_replace_resource_header();
	if(used) { 
		puts("wrote my.res.h");
	}
	else {
		puts("no change");
	}
    puts("building resource loader...");
    build_resource_loader(&pf);

	if(args.pack) {
		puts("--pack: generating res_release.c...");
		generate_file_res_release_c(&pf);
	}

	printf("done! in %fms", bt_passed_ms(time));
}