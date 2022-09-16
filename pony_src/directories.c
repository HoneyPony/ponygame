#include <sys/types.h>
#include <sys/stat.h>

void ensure_directory_tree_exists() {
	mkdir(".ponygame");
}