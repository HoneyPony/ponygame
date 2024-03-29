#include <stdio.h>
#include <string.h>

// Define all these variables that are needed for tracking tests.
int tests_total;
int tests_in_set;

int tests_passed_total;
int tests_passed_in_set;

int tests_failed_total;
int tests_failed_in_set;

// Define the functions used to communicate about tests...
void test_set_begin() {
	tests_in_set = 0;
	tests_passed_in_set = 0;
	tests_failed_in_set = 0;
}

void test_set_end(const char *name) {
	printf("%s:\n\ttotal tests: %d\n"
		"\ttests passed: %d\n\ttests failed: %d\n\n",
		name,
		tests_in_set,
		tests_passed_in_set,
		tests_failed_in_set);
}

// Define the various test sets that will be run in the tester.
void test_set_list();
void test_set_transforms();
void test_set_node();
void test_set_node_ref();

void bench_node_construct_destruct();
void bench_node_construct_destruct_small_batches();

// Other infrastructure
extern void pony_init_builtin_nodes();

int main(int argc, char **argv) {
	// Need to do some initialization to use nodes.
	// TODO: Use fork() or something? So each test set could do its
	// own initialization, to test internal node stuff?
	pony_init_builtin_nodes();

	if(argc > 1 && !strcmp(argv[1], "bench")) {
		bench_node_construct_destruct_small_batches();
		return 0;
	}

	test_set_list();
	test_set_transforms();
	test_set_node();
	test_set_node_ref();

	puts("\nall tests completed.");
	printf("overall:\n\ttotal tests: %d\n"
		"\ttests passed: %d\n\ttests failed: %d\n",
		tests_total,
		tests_passed_total,
		tests_failed_total);
}