#include <dirent.h>
#include <unistd.h>

#include <stdio.h>

#include "pony.h"

void check_path(const char *path, const char *file_name, DirTree *root_tree, ProjectFiles *output);

void check_directory(const char *root_path, const char *file_name, DIR* dir, DirTree *parent_tree, ProjectFiles *output) {
    struct dirent *dir_entry;

    DirTree child;
    child.type = DIR_TREE_DIRECTORY;
    child.name = str_from(file_name);
    ls_init(child.children);

    while ((dir_entry = readdir(dir)) != NULL) {
        if(!strcmp(dir_entry->d_name, ".")) continue;
        if(!strcmp(dir_entry->d_name, "..")) continue;

        str path = str_from(root_path);
        str child_file_name = str_from(dir_entry->d_name);

        str_push(path, '/');
        str_append_cstr(path, dir_entry->d_name);
        check_path(path, child_file_name, &child, output);

        str_free(child_file_name);
        str_free(path);
    }
    closedir(dir);

    ls_push(parent_tree->children, child);
}

void check_path(const char *path, const char *file_name, DirTree *root_path, ProjectFiles *output) {
    DIR *dir;

    dir = opendir(path);

    // Do not scan the ponygame path. (?)
    // if(cstr_has_prefix(path, "./.ponygame")) return;
    // Do not scan any . paths
    if(cstr_has_prefix(path, "./.")) return;
    // If the path is a directory, scan for files inside the directory.
    if (dir) {
        check_directory(path, file_name, dir, root_path, output);
        return;
    }

    closedir(dir);

    if(cstr_has_suffix(file_name, ".tex")) {
        DirTree result = {0};
        result.name = str_from(file_name);
        result.type = DIR_TREE_TEX;

        ls_init(result.tex_infos);
        ls_init(result.anim_infos);
        load_tex_file(path, &result.tex_infos, &result.anim_infos);

        foreach_r(tex, result.tex_infos, ls_push(output->tex_list, tex);)
        foreach_r(anim, result.anim_infos, ls_push(output->anim_list, anim);)

        ls_push(root_path->children, result);
    }

}

void get_dir_tree(ProjectFiles *output) {
    DirTree root;
    root.type = DIR_TREE_DIRECTORY;
    ls_init(root.children);

    check_path(".", "", &root, output);

    // Due to a strange quirk of how this is designed at the moment, the dir
    // tree algorithm reads the root directory into a child of itself.
    // Here is a quick fix...
    DirTree actual_root = root.children[0];
    ls_free(root.children);

    output->tree = actual_root;
}

// test code

void test_2(DirTree dt, int depth) {
    for(int i = 0; i < depth; ++i)
        printf("    ");
    printf("[%s] has type %d\n", dt.name, dt.type);
    if(dt.type == DIR_TREE_DIRECTORY) {
        foreach(child, dt.children, {
            test_2(child, depth + 1);
        })
    }
}

void test_dir_tree() {
    //DirTree dt = get_dir_tree();
    //test_2(dt, 0);
}
