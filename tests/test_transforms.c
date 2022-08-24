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

test_define(test_transform_scale_local_to_global, {
	Node *center = node_new(Node);

	set_lscale(center, vxy(3.0, 2.0));

	vec2 a = local_to_global(center, vxy(1.0, 1.0));
	vec2 b = local_to_global(center, vxy(-1.0, -2.0));
	vec2 c = local_to_global(center, vxy(3.5, -2.25));

	test_assert(equals(a, vxy(3.0, 2.0)),   "expected  3.0,  2.0 got %f, %f\n", a.x, a.y);
	test_assert(equals(b, vxy(-3.0, -4.0)), "expected -3.0, -4.0 got %f, %f\n", b.x, b.y);
	test_assert(equals(c, vxy(10.5, -4.5)), "expected 10.5, -4.5 got %f, %f\n", c.x, c.y);
})

test_define(test_transform_set_gpos_no_nudge, {
	// When testing the global position, it is important to call the local
	// translate function, as the most important thing to test is the interaction
	// between global coordinates and local coordinates.
	//
	// However, one simpler thing to test is whether set_gpos and get_gpos
	// make the round trip at all. They should very easily round trip, if no
	// changes are made to the local position. But it's a good way to make sure
	// there's nothing more fundamentally wrong when starting to debug the
	// actual problems with set_gpos and how it interacts with local coordinates.
	Node *a = node_new(Node);
	Node *b = node_new(Node);
	Node *c = node_new(Node);

	reparent(b, a);
	reparent(c, b);

	set_lpos(a, vxy(2.0, 3.0));
	set_lpos(b, vxy(-10.0, 9.0));

	set_gpos(c, vxy(10.0, 10.0));
	//ltranslate(c, vxy(1.0, 0.0));
	//ltranslate(c, vxy(-1.0, 0.0));

	vec2 gpos = get_gpos(c);
	test_assert(equals(gpos, vxy(10.0, 10.0)), "expected 10.0, 10.0 got %f, %f\n", gpos.x, gpos.y);
})

test_define(test_transform_set_gpos_in_offset_heirarchy, {
	Node *a = node_new(Node);
	Node *b = node_new(Node);
	Node *c = node_new(Node);

	reparent(b, a);
	reparent(c, b);

	set_lpos(a, vxy(2.0, 3.0));
	set_lpos(b, vxy(-10.0, 9.0));

	set_gpos(c, vxy(10.0, 10.0));
	ltranslate(c, vxy(1.0, 0.0));
	ltranslate(c, vxy(-1.0, 0.0));

	vec2 gpos = get_gpos(c);
	test_assert(equals(gpos, vxy(10.0, 10.0)), "expected 10.0, 10.0 got %f, %f\n", gpos.x, gpos.y);
})

test_define(test_transform_set_gpos_in_somewhat_complex_heirarchy, {
	Node *a = node_new(Node);
	Node *b = node_new(Node);
	Node *c = node_new(Node);

	reparent(b, a);
	reparent(c, b);

	set_lscale(a, vxy(2.0, 2.0));
	set_lrot_deg(a, 45.0);

	set_lpos(b, vxy(5.0, 0.0));
	set_lrot_deg(b, 45.0);

	set_gpos(c, vxy(10.0, 10.0));
	ltranslate(c, vxy(1.0, 0.0));
	vec2 test = get_gpos(c);
	printf("Note: somewhat complex hierarchy position after nudge: %f, %f\n", test.x, test.y);
	ltranslate(c, vxy(-1.0, 0.0));

	vec2 gpos = get_gpos(c);
	test_assert(equals(gpos, vxy(10.0, 10.0)), "expected 10.0, 10.0 got %f, %f\n", gpos.x, gpos.y);
})

test_define(test_transform_set_gpos_in_complex_heirarchy, {
	Node *a = node_new(Node);
	Node *b = node_new(Node);
	Node *c = node_new(Node);
	Node *d = node_new(Node);

	reparent(b, a);
	reparent(c, b);
	reparent(d, c);

	set_lscale(a, vxy(2.0, 2.0));
	set_lrot_deg(a, 3.0);

	set_lpos(b, vxy(5.0, 0.0));
	set_lrot_deg(b, 3.0);

	set_lpos(c, vxy(-2.0, 5.0));
	set_lscale(c, vxy(1.2, 1.2));
	set_lrot_deg(c, 8.0);

	set_gpos(d, vxy(10.0, 10.0));
	ltranslate(d, vxy(1.0, 0.0));
	vec2 test = get_gpos(c);
	printf("Note: complex hierarchy position after nudge: %f, %f\n", test.x, test.y);
	ltranslate(d, vxy(-1.0, 0.0));

	vec2 gpos = get_gpos(d);
	test_assert(equals(gpos, vxy(10.0, 10.0)), "expected 10.0, 10.0 got %f, %f\n", gpos.x, gpos.y);
})

test_set_define(test_set_transforms, {
	test_transform_basic_offset();
	test_transform_45_deg_parent();
	test_transform_scale_local_to_global();
	test_transform_set_gpos_no_nudge();
	test_transform_set_gpos_in_offset_heirarchy();
	test_transform_set_gpos_in_somewhat_complex_heirarchy();
	test_transform_set_gpos_in_complex_heirarchy();
})