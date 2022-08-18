#include "test.h"

#include "pony_list.h"

test_define(test_list_push, {
	list_of(int) list;
	ls_init(list);

	for(int i = 0; i < 20; ++i) {
		ls_push(list, i);
		test_assert_eq((uint32_t)(i + 1), ls_length(list), "Wrong length after push. Expected %d got %d.");
		test_assert_eq(i, list[i], "Wrong pushed element. Expected %d got %d.");
	}
})

test_define(test_list_empty, {
	list_of(int) list;
	ls_init(list);

	test_assert(ls_empty(list), "List not empty when initialized.");
	
	ls_push(list, 5);
	ls_push(list, 10);

	test_assert(!ls_empty(list), "List reported empty when it is not.");

	ls_pop(list);
	ls_pop(list);

	test_assert(ls_empty(list), "List not empty when it should be.");
})

test_set_define(test_set_list, {
	test_list_push();
	test_list_empty();
})