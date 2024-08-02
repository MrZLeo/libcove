#include <criterion/assert.h>
#include <criterion/criterion.h>

#include "rbtree.h"
#include "tools.h"

// Define a structure that includes rb_node
struct my_node {
    struct rb_node rb;
    int value;
};

// Helper function to create a new node
static struct my_node *create_node(int value) {
    struct my_node *node = malloc(sizeof(struct my_node));
    node->value = value;
    node->rb.__rb_parent_color = RB_RED;
    node->rb.rb_left = NULL;
    node->rb.rb_right = NULL;
    return node;
}

[[gnu::always_inline]]
static inline bool my_node_less(
    const struct rb_node *node,
    const struct rb_node *other
) {
    return rb_entry(node, struct my_node, rb)->value
        < rb_entry(other, struct my_node, rb)->value;
}

// Test insertion of a single node
Test(rb_insert_color, single_node) {
    struct rb_root root = { NULL };
    struct my_node *node = create_node(10);

    rb_add(&root, &node->rb, my_node_less);

    cr_assert_eq(
        root.rb_node,
        &node->rb,
        "Root node should be the inserted node"
    );
    cr_assert(rb_is_black(&node->rb), "Root node should be black");

    free(node);
}

// Test insertion that don't need to change colors
Test(rb_insert_color, parent_is_balck) {
    struct rb_root root = { NULL };
    struct my_node *node1 = create_node(10);
    struct my_node *node2 = create_node(20);

    rb_add(&root, &node1->rb, my_node_less);
    rb_add(&root, &node2->rb, my_node_less);

    cr_assert_eq(
        root.rb_node,
        &node1->rb,
        "Root node should be the first inserted node"
    );
    cr_assert(rb_is_black(&node1->rb), "Root node should be black");
    cr_assert_eq(
        node1->rb.rb_right,
        &node2->rb,
        "Second node should be the right child of the root"
    );
    cr_assert(rb_is_red(&node2->rb), "Second node should be red");

    free(node1);
    free(node2);
}

// Test insertion that causes a RR Inbalance
Test(rb_insert_color, rr_inbalance) {
    struct rb_root root = { NULL };
    struct my_node *node1 = create_node(10);
    struct my_node *node2 = create_node(20);
    struct my_node *node3 = create_node(30);

    rb_add(&root, &node1->rb, my_node_less);
    rb_add(&root, &node2->rb, my_node_less);
    rb_add(&root, &node3->rb, my_node_less);

    cr_assert_eq(
        root.rb_node,
        &node2->rb,
        "Root node should be the second inserted node"
    );
    cr_assert_eq(
        node2->rb.rb_left,
        &node1->rb,
        "First node should be the left child of the root"
    );
    cr_assert_eq(
        node2->rb.rb_right,
        &node3->rb,
        "Third node should be the right child of the root"
    );

    struct rb_node end;
    __auto_type null_nodes = (struct rb_node *[]) {
        node1->rb.rb_left,
        node1->rb.rb_right,
        node3->rb.rb_left,
        node3->rb.rb_right,
        &end,
    };
    for (__auto_type iter = null_nodes; *iter != &end; iter++)
        cr_assert_eq(NULL, *iter, "All leaf nodes should be NULL");

    cr_assert(rb_is_black(&(node2->rb)), "Root node should be black");
    cr_assert(rb_is_red(&(node1->rb)), "Node1 should be red");
    cr_assert(rb_is_red(&(node3->rb)), "Node3 should be red");

    free(node1);
    free(node2);
    free(node3);
}

// Test insertion that causes a LR Inbalance
Test(rb_insert_color, lr_inbalance) {
    struct rb_root root = { NULL };
    struct my_node *node1 = create_node(30);
    struct my_node *node2 = create_node(10);
    struct my_node *node3 = create_node(20);

    rb_add(&root, &node1->rb, my_node_less);
    rb_add(&root, &node2->rb, my_node_less);
    rb_add(&root, &node3->rb, my_node_less);

    cr_assert_eq(
        root.rb_node,
        &node3->rb,
        "Root node should be the third inserted node"
    );
    cr_assert_eq(
        node3->rb.rb_left,
        &node2->rb,
        "Second node should be the left child of the root"
    );
    cr_assert_eq(
        node3->rb.rb_right,
        &node1->rb,
        "First node should be the right child of the root"
    );

    struct rb_node end;
    __auto_type null_nodes = (struct rb_node *[]) {
        node2->rb.rb_left,
        node2->rb.rb_right,
        node1->rb.rb_left,
        node1->rb.rb_right,
        &end,
    };
    for (__auto_type iter = null_nodes; *iter != &end; iter++)
        cr_assert_eq(NULL, *iter, "All leaf nodes should be NULL");

    cr_assert(rb_is_black(&(node3->rb)), "Root node should be black");
    cr_assert(rb_is_red(&(node2->rb)), "Node2 should be red");
    cr_assert(rb_is_red(&(node1->rb)), "Node1 should be red");

    free(node1);
    free(node2);
    free(node3);
}

// Test insertion that causes a LL Inbalance
Test(rb_insert_color, ll_inbalance) {
    struct rb_root root = { NULL };
    struct my_node *node1 = create_node(30);
    struct my_node *node2 = create_node(20);
    struct my_node *node3 = create_node(10);

    rb_add(&root, &node1->rb, my_node_less);
    rb_add(&root, &node2->rb, my_node_less);
    rb_add(&root, &node3->rb, my_node_less);

    cr_assert_eq(
        root.rb_node,
        &node2->rb,
        "Root node should be the second inserted node"
    );
    cr_assert_eq(
        node2->rb.rb_left,
        &node3->rb,
        "Third node should be the left child of the root"
    );
    cr_assert_eq(
        node2->rb.rb_right,
        &node1->rb,
        "First node should be the right child of the root"
    );

    struct rb_node end;
    __auto_type null_nodes = (struct rb_node *[]) {
        node3->rb.rb_left,
        node3->rb.rb_right,
        node1->rb.rb_left,
        node1->rb.rb_right,
        &end,
    };
    for (__auto_type iter = null_nodes; *iter != &end; iter++)
        cr_assert_eq(NULL, *iter, "All leaf nodes should be NULL");

    cr_assert(rb_is_black(&(node2->rb)), "Root node should be black");
    cr_assert(rb_is_red(&(node3->rb)), "Node3 should be red");
    cr_assert(rb_is_red(&(node1->rb)), "Node1 should be red");

    free(node1);
    free(node2);
    free(node3);
}

// Test insertion that causes a RL Inbalance
Test(rb_insert_color, rl_inbalance) {
    struct rb_root root = { NULL };
    struct my_node *node1 = create_node(10);
    struct my_node *node2 = create_node(30);
    struct my_node *node3 = create_node(20);

    rb_add(&root, &node1->rb, my_node_less);
    rb_add(&root, &node2->rb, my_node_less);
    rb_add(&root, &node3->rb, my_node_less);

    cr_assert_eq(
        root.rb_node,
        &node3->rb,
        "Root node should be the third inserted node"
    );
    cr_assert_eq(
        node3->rb.rb_left,
        &node1->rb,
        "First node should be the left child of the root"
    );
    cr_assert_eq(
        node3->rb.rb_right,
        &node2->rb,
        "Second node should be the right child of the root"
    );

    struct rb_node end;
    __auto_type null_nodes = (struct rb_node *[]) {
        node1->rb.rb_left,
        node1->rb.rb_right,
        node2->rb.rb_left,
        node2->rb.rb_right,
        &end,
    };
    for (__auto_type iter = null_nodes; *iter != &end; iter++)
        cr_assert_eq(NULL, *iter, "All leaf nodes should be NULL");

    cr_assert(rb_is_black(&(node3->rb)), "Root node should be black");
    cr_assert(rb_is_red(&(node1->rb)), "Node1 should be red");
    cr_assert(rb_is_red(&(node2->rb)), "Node2 should be red");

    free(node1);
    free(node2);
    free(node3);
}

// Test insertion that causes a case1
Test(rb_insert_color, case1) {
    struct rb_root root = { NULL };
    struct my_node *node1 = create_node(20);
    struct my_node *node2 = create_node(10);
    struct my_node *node3 = create_node(30);
    struct my_node *node4 = create_node(5);

    rb_add(&root, &node1->rb, my_node_less);
    rb_add(&root, &node2->rb, my_node_less);
    rb_add(&root, &node3->rb, my_node_less);
    rb_add(&root, &node4->rb, my_node_less);

    cr_assert_eq(
        root.rb_node,
        &node1->rb,
        "Root node should be the first inserted node"
    );
    cr_assert_eq(
        node1->rb.rb_left,
        &node2->rb,
        "Second node should be the left child of the root"
    );
    cr_assert_eq(
        node1->rb.rb_right,
        &node3->rb,
        "Third node should be the right child of the root"
    );
    cr_assert_eq(
        node2->rb.rb_left,
        &node4->rb,
        "Forth node should be the left child of the node2"
    );

    struct rb_node end;
    __auto_type null_nodes = (struct rb_node *[]) {
        node2->rb.rb_right, node3->rb.rb_left,  node3->rb.rb_right,
        node4->rb.rb_left,  node4->rb.rb_right, &end,
    };
    for (__auto_type iter = null_nodes; *iter != &end; iter++)
        cr_assert_eq(NULL, *iter, "All leaf nodes should be NULL");

    cr_assert(rb_is_black(&(node1->rb)), "Root node should be black");
    cr_assert(rb_is_black(&(node2->rb)), "Node2 should be black");
    cr_assert(rb_is_black(&(node3->rb)), "Node3 should be black");
    cr_assert(rb_is_red(&(node4->rb)), "Node4 should be red");

    free(node1);
    free(node2);
    free(node3);
    free(node4);
}

// Test another insertion that causes a case1
Test(rb_insert_color, case1_other) {
    struct rb_root root = { NULL };
    struct my_node *node1 = create_node(20);
    struct my_node *node2 = create_node(10);
    struct my_node *node3 = create_node(30);
    struct my_node *node4 = create_node(35);

    rb_add(&root, &node1->rb, my_node_less);
    rb_add(&root, &node2->rb, my_node_less);
    rb_add(&root, &node3->rb, my_node_less);
    rb_add(&root, &node4->rb, my_node_less);

    cr_assert_eq(
        root.rb_node,
        &node1->rb,
        "Root node should be the first inserted node"
    );
    cr_assert_eq(
        node1->rb.rb_left,
        &node2->rb,
        "Second node should be the left child of the root"
    );
    cr_assert_eq(
        node1->rb.rb_right,
        &node3->rb,
        "Third node should be the right child of the root"
    );
    cr_assert_eq(
        node3->rb.rb_right,
        &node4->rb,
        "Forth node should be the left child of the node3"
    );

    struct rb_node end;
    __auto_type null_nodes = (struct rb_node *[]) {
        node2->rb.rb_left, node2->rb.rb_right, node3->rb.rb_left,
        node4->rb.rb_left, node4->rb.rb_right, &end,
    };
    for (__auto_type iter = null_nodes; *iter != &end; iter++)
        cr_assert_eq(NULL, *iter, "All leaf nodes should be NULL");

    cr_assert(rb_is_black(&(node1->rb)), "Root node should be black");
    cr_assert(rb_is_black(&(node2->rb)), "Node2 should be black");
    cr_assert(rb_is_black(&(node3->rb)), "Node3 should be black");
    cr_assert(rb_is_red(&(node4->rb)), "Node4 should be red");

    free(node1);
    free(node2);
    free(node3);
    free(node4);
}

// Test erasing a single node
Test(rb_erase, single_node) {
    struct rb_root root = { NULL };
    struct my_node *node = create_node(10);

    rb_add(&root, &node->rb, my_node_less);
    rb_erase(&root, &node->rb);

    cr_assert_eq(
        root.rb_node,
        NULL,
        "Root node should be NULL after erasing the only node"
    );

    free(node);
}

// Test erasing a single node
Test(rb_erase, erase_all) {
    struct rb_root root = { NULL };
    struct my_node *node1 = create_node(10);
    struct my_node *node2 = create_node(20);
    struct my_node *node3 = create_node(30);
    struct my_node *node4 = create_node(40);
    struct my_node *node5 = create_node(50);
    struct my_node *node6 = create_node(60);

    rb_add(&root, &node1->rb, my_node_less);
    rb_add(&root, &node2->rb, my_node_less);
    rb_add(&root, &node3->rb, my_node_less);
    rb_add(&root, &node4->rb, my_node_less);
    rb_add(&root, &node5->rb, my_node_less);
    rb_add(&root, &node6->rb, my_node_less);
    rb_erase(&root, &node1->rb);
    rb_erase(&root, &node2->rb);
    rb_erase(&root, &node3->rb);
    rb_erase(&root, &node4->rb);
    rb_erase(&root, &node5->rb);
    rb_erase(&root, &node6->rb);

    cr_assert_eq(
        root.rb_node,
        NULL,
        "Root node should be NULL after erasing all node"
    );

    free(node1);
    free(node2);
    free(node3);
    free(node4);
    free(node5);
    free(node6);
}

// Test erasing a node with one child
Test(rb_erase, node_with_one_child) {
    struct rb_root root = { NULL };
    struct my_node *node1 = create_node(10);
    struct my_node *node2 = create_node(20);

    rb_add(&root, &node1->rb, my_node_less);
    rb_add(&root, &node2->rb, my_node_less);
    rb_erase(&root, &node1->rb);

    cr_assert_eq(
        root.rb_node,
        &node2->rb,
        "Root node should be the second inserted node"
    );
    cr_assert(rb_is_black(&node2->rb), "Root node should be black");

    free(node1);
    free(node2);
}

// Test erasing a node with two children
Test(rb_erase, node_with_two_children) {
    struct rb_root root = { NULL };
    struct my_node *node1 = create_node(10);
    struct my_node *node2 = create_node(20);
    struct my_node *node3 = create_node(30);

    rb_add(&root, &node1->rb, my_node_less);
    rb_add(&root, &node2->rb, my_node_less);
    rb_add(&root, &node3->rb, my_node_less);
    rb_erase(&root, &node2->rb);

    cr_assert_eq(
        root.rb_node,
        &node3->rb,
        "Root node should be the first inserted node"
    );
    cr_assert_eq(
        node3->rb.rb_left,
        &node1->rb,
        "Third node should be the right child of the root"
    );

    free(node1);
    free(node2);
    free(node3);
}

// Test erasing the root node
Test(rb_erase, erase_root_node) {
    struct rb_root root = { NULL };
    struct my_node *node1 = create_node(10);
    struct my_node *node2 = create_node(20);

    rb_add(&root, &node1->rb, my_node_less);
    rb_add(&root, &node2->rb, my_node_less);
    rb_erase(&root, &node1->rb);

    cr_assert_eq(
        root.rb_node,
        &node2->rb,
        "Root node should be the second inserted node"
    );
    cr_assert(rb_is_black(&node2->rb), "Root node should be black");

    free(node1);
    free(node2);
}

// Test erasing a node that causes rebalancing
Test(rb_erase, rebalancing) {
    struct rb_root root = { NULL };
    struct my_node *node1 = create_node(10);
    struct my_node *node2 = create_node(20);
    struct my_node *node3 = create_node(30);

    rb_add(&root, &node1->rb, my_node_less);
    rb_add(&root, &node2->rb, my_node_less);
    rb_add(&root, &node3->rb, my_node_less);
    rb_erase(&root, &node1->rb);

    cr_assert_eq(
        root.rb_node,
        &node2->rb,
        "Root node should be the second inserted node"
    );
    cr_assert_eq(
        node2->rb.rb_right,
        &node3->rb,
        "Third node should be the right child of the root"
    );

    free(node1);
    free(node2);
    free(node3);
}
