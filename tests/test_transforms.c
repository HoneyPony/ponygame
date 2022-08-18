#include "test.h"

#include "pony_node.h"

test_define(test_transform_basic_offset, {
	Node *top = node_new(Node);
	Node *mid = node_new(Node);
	Node *child = node_new(Node);

	reparent(child, mid);
	reparent(mid, top);

	set_lpos(top, vxy(5, 3));
	set_lpos(mid, vxy(7, 2));
	set_lpos(child, vxy(-2, -7));

	vec2 gpos = get_gpos(child);

	test_assert_eq(10.0, gpos.x, "Wrong child x position. Expected %f got %f");
	test_assert_eq(-2.0, gpos.y, "Wrong child y position. Expected %f got %f");
})

test_define(test_transform_45_deg_parent, {
	Node *parent = node_new(Node);
	Node *child = node_new(Node);

	reparent(child, parent);

	set_lpos(parent, vxy(2.0, 2.0));
	set_lpos(child, vxy(2.0, 0.0));

	set_lrot_deg(parent, 45.0);

	vec2 gpos = get_gpos(child);

	test_assert(gpos.x == gpos.y, "Child x should equal y. Instead, x = %f, y = %f", gpos.x, gpos.y)
})

test_set_define(test_set_transforms, {
	test_transform_basic_offset();
	test_transform_45_deg_parent();
})