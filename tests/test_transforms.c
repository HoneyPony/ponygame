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

	test_assert(gpos.x == gpos.y, "Child x should equal y. Instead, x = %f, y = %f", gpos.x, gpos.y);
})

// TODO: Implement vector math
bool equals(vec2 a, vec2 b) {
	return (a.x == b.x) && (a.y == b.y);
}

test_define(test_scale_local_to_global, {
	Node *center = node_new(Node);

	set_lscale(center, vxy(3.0, 2.0));

	vec2 a = local_to_global(center, vxy(1.0, 1.0));
	vec2 b = local_to_global(center, vxy(-1.0, -2.0));
	vec2 c = local_to_global(center, vxy(3.5, -2.25));

	test_assert(equals(a, vxy(3.0, 2.0)),   "expected  3.0,  2.0 got %f, %f\n", a.x, a.y);
	test_assert(equals(b, vxy(-3.0, -4.0)), "expected -3.0, -4.0 got %f, %f\n", b.x, b.y);
	test_assert(equals(c, vxy(10.5, -4.5)), "expected 10.5, -4.5 got %f, %f\n", c.x, c.y);
})

test_set_define(test_set_transforms, {
	test_transform_basic_offset();
	test_transform_45_deg_parent();
	test_scale_local_to_global();
})