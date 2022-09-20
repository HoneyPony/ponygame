#include <stdio.h>

#include "pony.h"

void save_path_list(PathList *list) {
	ensure_directory_tree_exists();
	FILE *out = fopen(".ponygame/my.ponyfiles", "w");

	fputs("[files:c]\n", out);

	foreach(path, list->c_paths, {
		fputs(path, out);
		fputc('\n', out);
	})

	fputs("[files:tex]\n", out);

	foreach(path, list->tex_paths, {
		fputs(path, out);
		fputc('\n', out);
	})

	fputs("[files:pony]\n", out);

	foreach(path, list->pony_paths, {
		fputs(path, out);
		fputc('\n', out);
	})
}

void make_ninja_file(PathList *list, Config *config) {
	list_of(TexBuildInfo) tex_build;
	ls_init(tex_build);
	foreach(path, list->tex_paths, {
		TexBuildInfo build = load_tex_build_info(path);
		ls_push_var(tex_build, build);
	})

	FILE *out = fopen("build.ninja", "w");

	fputs("builddir = .ponygame/build\n", out);
	fputs("cc = gcc\n", out);
	fputs("cflags = -g -Wall -O1 ", out);
	foreach(path, config->include_paths, {
		fprintf(out, "-I%s ", path);
	})
	fputs("\nldflags = \n", out);
	fputs("libs = \n\n", out);

	fputs("rule cc\n", out);
	fputs("  command = $cc -MMD -MT $out -MF $out.d $cflags -c $in -o $out\n", out);
	fputs("  depfile = $out.d\n", out);
	fputs("  deps = gcc\n\n", out);
	
	fputs("rule link\n", out);
	fputs("  command = $cc $ldflags -o $out $in $libs\n\n", out);

	fputs("rule tex\n", out);
	fputs("  command = pony tex $in\n\n", out);

	// Build the pre-compiled header
	fputs("build .ponygame/my.ponygame.h.pch: cc .ponygame/my.ponygame.h\n\n", out);

	foreach(cfile, list->c_paths, {
		fprintf(out, "build $builddir/%s.o: cc %s\n", cfile, cfile);
	})

	fputs("\n", out);
	
	fputs("build .ponygame/tex.lock: tex ", out);
	foreach(path, list->tex_paths, {
		fprintf(out, "%s ", path);
	})
	foreach(info, tex_build, {
		if(info.image_source) fprintf(out, "%s ", info.image_source);
		if(info.aseprite_source) fprintf(out, "%s ", info.aseprite_source);
	})
}