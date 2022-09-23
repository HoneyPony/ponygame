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

void ninja_c_to_o(FILE *out, const char *cfile) {
	fprintf(out, "build $builddir/%s.o: cc %s\n", cfile, cfile);
}

void ninja_include_object(FILE *out, const char *cfile) {
	fprintf(out, "$builddir/%s.o ", cfile);
}

str ninja_combo_name(bool is_release, bool is_web, const char *postfix) {
	str name = str_from("build.");
	str_append_cstr(name, is_release ? "release." : "debug.");
	str_append_cstr(name, is_web ? "web" : "local");
	str_append_cstr(name, postfix);
	return name;
}

void make_ninja_file(PathList *list, Config *config, bool is_release, bool is_web) {
	list_of(TexBuildInfo) tex_build;
	ls_init(tex_build);
	foreach(path, list->tex_paths, {
		TexBuildInfo build = load_tex_build_info(path);
		ls_push_var(tex_build, build);
	})

	str fname = ninja_combo_name(is_release, is_web, ".ninja");

	FILE *out = fopen(fname, "w");
	

	str buildname = ninja_combo_name(is_release, is_web, "");
	fprintf(out, "builddir = .ponygame/%s\n", buildname);
	str_free(buildname);

	const char *compiler = "gcc";
	if(is_web) compiler = config->emcc;
	fprintf(out, "cc = %s\n", compiler);
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
	if(is_release) {
		fputs("  command = pony scan --pack\n  restat = true\n\n", out);
	}
	else {
		fputs("  command = pony scan\n  restat = true\n\n", out);
	}

	fputs("rule gen\n", out);
	fputs("  command = pony generate:$fname\n\n", out);

	// Build the pre-compiled header
	if(!is_web) {
		fputs("build .ponygame/my.ponygame.h.pch: cc .ponygame/my.ponygame.h\n\n", out);
	}

	// Build resource debug file
	if(is_release) {
		ninja_c_to_o(out, ".ponygame/res_release.c");
		fputs("build .ponygame/res_release.c: scan\n", out);
	}
	else {
		ninja_c_to_o(out, ".ponygame/res_debug.c");
		fputs("build .ponygame/res_debug.c: gen\n  fname = res_debug.c\n", out);
	}
	// Build the resource loader file
	ninja_c_to_o(out, ".ponygame/res_loader.c");
	// Build user C files
	foreach(cfile, list->c_paths, {
		fprintf(out, "build $builddir/%s.o: cc %s\n", cfile, cfile);
	})

	fputs("\nbuild game.exe: link ", out);
	// Include res loader object files
	ninja_include_object(out, ".ponygame/res_loader.c");
	if(is_release) {
		ninja_include_object(out, ".ponygame/res_release.c");
	}
	else {
		ninja_include_object(out, ".ponygame/res_debug.c");
	}
	// Include user object files
	foreach(cfile, list->c_paths, {
		ninja_include_object(out, cfile);
    })
	
    fputs("| ", out);
    if(config->lib_file) {
        fprintf(out, "%s ", config->lib_file);
    }
    // Library paths...
	if(is_web) {
		fputs("\n  libs = -sUSE_SDL=2 -sUSE_SDL_MIXER=2 -lponygame_web\n", out);
	}
	else {
		if(is_release) {
			// Build a statically linked executable
			fputs("\n  libs = "
			"-lmingw32 -lSDL2main -lponygame -lSDL2 -lSDL2_mixer -lglew32 -lopengl32 "

			"-static -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm "
			"-limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc -lsetupapi "
			"-lmpg123 -lopusfile -lopus -logg -lssp "
			"-lshlwapi "
			"\n", out);
		}
		else {
   			fputs("\n  libs = -lmingw32 -lSDL2main -lponygame -lSDL2 -lSDL2_mixer -lglew32 -lopengl32 -lgdi32\n", out);
		}
	}

	foreach(info, tex_build, {
		// TODO: Figure out if there needs to be some special handling for nested
		// tex paths...?
		if(info.aseprite_source && info.image_source && info.tex_path) {
			fprintf(out, "build %s | %s: aseprite %s\n", info.tex_path, info.image_source, info.aseprite_source);
		}
	})
    
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
	//
	// Solution: It seems that adding restat = true to the 'scan' rule makes this
	// work correctly. ninja figures out that my.res.h is up to date, and doesn't
	// bother to try to rebuild it.
	fprintf(out, "\n\nbuild %s .ponygame/my.res.h: scan | ", fname);
	foreach(path, list->tex_paths, {
		fprintf(out, "%s ", path);
	})
	foreach(info, tex_build, {
		if(info.image_source) fprintf(out, "%s ", info.image_source);
	})
	fputs("\n", out); // Need a newline at the end of the ninja file

	str_free(fname);
}