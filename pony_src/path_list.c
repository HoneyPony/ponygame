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
	fputs("cflags = -g -Wall -O1 -I.ponygame ", out);
	foreach(path, config->include_paths, {
		fprintf(out, "-I%s ", path);
	})
	fputs("\nldflags = ", out);
    foreach(path, config->lib_paths, {
        fprintf(out, "-L%s ", path);
    })
	fputs("\nlibs = \n\n", out);

	fputs("rule cc\n", out);
	fputs("  command = $cc -MMD -MT $out -MF $out.d $cflags -c $in -o $out\n", out);
	fputs("  depfile = $out.d\n", out);
	fputs("  deps = gcc\n\n", out);
	
	fputs("rule link\n", out);
	fputs("  command = $cc $ldflags -o $out $in $libs\n\n", out);

	fputs("rule aseprite\n", out);
	fputs("  command = pony tex-from-aseprite $out\n\n", out);

	fputs("rule scan\n", out);
	fputs("  command = pony scan\n\n", out);

	// The build.ninja file is rebuilt whenever we scan. As such, we don't need
	// any additional outputs.. Also, ninja (probably?) needs to know that this
	// command affects its build file...
	//
	// Also, we need to do this *before* building any affected C files.
	//
	// However, adding my.res.h as an output causes an infinite loop, because
	// ninja thinks it is always out of date (when the whole point is that sometimes
	// it is out of date...)
	//
	// So it isn't clear what the best solution is. Perhaps, just put this task
	// at the top of the file and hope that it works out?
	fputs("\n\nbuild build.ninja: scan | ", out);
	foreach(path, list->tex_paths, {
		fprintf(out, "%s ", path);
	})
	foreach(info, tex_build, {
		if(info.image_source) fprintf(out, "%s ", info.image_source);
	})
	fputs("\n", out); // Need a newline at the end of the ninja file

	// Build the pre-compiled header
	fputs("build .ponygame/my.ponygame.h.pch: cc .ponygame/my.ponygame.h\n\n", out);

	// Build the resource loader file
	fputs("build .ponygame/res_loader.c.o: cc .ponygame/res_loader.c\n", out);
	// Build user C files
	foreach(cfile, list->c_paths, {
		fprintf(out, "build $builddir/%s.o: cc %s\n", cfile, cfile);
	})

	fputs("\nbuild game.exe: link ", out);
	// Include res loader object file
    fputs(".ponygame/res_loader.c.o ", out);
	// Include user object files
	foreach(cfile, list->c_paths, {
        fprintf(out, "$builddir/%s.o ", cfile);
    })
	
    fputs("| ", out);
    if(config->lib_file) {
        fprintf(out, "%s ", config->lib_file);
    }
    // Library paths...
    fputs("\n  libs = -lmingw32 -lSDL2main -lponygame -lSDL2 -lglew32 -lopengl32 -lgdi32\n", out);

	foreach(info, tex_build, {
		// TODO: Figure out if there needs to be some special handling for nested
		// tex paths...?
		if(info.aseprite_source && info.image_source && info.tex_path) {
			fprintf(out, "build %s | %s: aseprite %s\n", info.tex_path, info.image_source, info.aseprite_source);
		}
	})

	
	/*fputs("\n\nbuild .ponygame/tex.lock: tex ", out);
	foreach(path, list->tex_paths, {
		fprintf(out, "%s ", path);
	})

    fputs("| ", out); // Image files imply tex rebuild, but are not part of the command line

	foreach(info, tex_build, {
		if(info.image_source) fprintf(out, "%s ", info.image_source);
		if(info.aseprite_source) fprintf(out, "%s ", info.aseprite_source);
	})*/
    
}