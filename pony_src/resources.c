#include <stdio.h>

#include "pony.h"

bool tex_file_is_static(DirTree *file) {
    return ls_empty(file->anim_infos) && (ls_length(file->tex_infos) == 1);
}

void build_tex_handle_statement(TexFileInfo *tf, FILE *out) {
    fprintf(out, "(TexHandle) {\n\t\tspritesheet%04d.\n", tf->spritesheet_index);

	printf("[%s] generated coords: %d, %d, %d, %d\n", tf->anim_name, tf->coords.sx, tf->coords.sy, tf->coords.ex, tf->coords.ey);

    float sx, sy, ex, ey;

    sx = (float)tf->coords.sx / 1024.0;
    ex = (float)tf->coords.ex / 1024.0;

    // Vertical flip..?
    sy = (1024.0 - (float)tf->coords.ey) / 1024.0;
    ey = (1024.0 - (float)tf->coords.sy) / 1024.0;

	printf("%s uvs = %f, %f -> %f, %f\n", tf->anim_name, sx, sy, ex, ey);

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
			fprintf(out, "\t%s%s.%s = (AnimFrame) {\n", prefix, sname, anim.name);
			fprintf(out, "\t\t%d,\n", anim.frame_count);
			fprintf(out, "\t\tframe_memory + %d\n", pf->anim_frame_ptr);
			fprintf(out, "\t};\n");

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
}

void resource_loader_allocate_frame_list(ProjectFiles *pf, FILE *out) {
	int frame_count = 0;
	foreach(anim, pf->anim_list, {
		frame_count += anim->frame_count;
	})

	fprintf(out, "static AnimFrame frame_memory[%d]\n\n", frame_count);
}

void build_resource_loader(ProjectFiles *pf) {

    FILE *out = fopen(FILE_NAME_RES_LOADER, "w");

    fputs("#include \"my.ponygame.h\"\n\n", out);

	resource_loader_allocate_frame_list(pf, out);
	pf->anim_frame_ptr = 0;

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

void build_resource_header_tree(ProjectFiles *pf, DirTree *tree, int depth, FILE *out) {
    if(tree->type == DIR_TREE_TEX) {
        build_tex_header(tree, depth, out);
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
    FILE *out = fopen(".ponygame/my.res.h", "w");
    fputs("#pragma once\n\n#include <ponygame.h>\n\n", out);

    fputs("struct res {\n", out);

    foreach(dt, pf->tree.children, {
        build_resource_header_tree(pf, &dt, 1, out);
    })

    fputs("} res;\n", out);

    fclose(out);
}

void rebuild_resources() {
    ProjectFiles pf;

    pf.path_list = scan_for_files();
    pf.config = load_config();

    ls_init(pf.tex_list);
    ls_init(pf.anim_list);

    //foreach(path, list.tex_paths, {
    //    load_tex_file(path, &tex_list, &anim_list);
    //})

    // Load the directory tree...
    get_dir_tree(&pf);

    puts("Packing textures...");
    pack_images(&pf);

    puts("Building resource header...");
    build_resource_header(&pf);
    puts("Building resource loader...");
    build_resource_loader(&pf);
}