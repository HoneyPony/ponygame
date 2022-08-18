#include <stdio.h>

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
		"\ttests passed: %d\n\ttests failed: %d\n",
		name,
		tests_in_set,
		tests_passed_in_set,
		tests_failed_in_set);
}

// Define the various test sets that will be run in the tester.
void test_set_list();

int main(int argc, char **argv) {
	test_set_list();

	puts("\nall tests completed.");
	printf("overall:\n\ttotal tests: %d\n"
		"\ttests passed: %d\n\ttests failed: %d\n",
		tests_total,
		tests_passed_total,
		tests_failed_total);
}