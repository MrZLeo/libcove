// SPDX-License-Identifier: GPL-2.0-only

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "rbtree.h"
#include "rbtree_augmented.h"
#include "unity.h"

// Simple PRNG wrapper
static inline uint32_t prandom_u32_state(void *state) {
    (void)state; // Unused in userspace version
    return (uint32_t)rand();
}


static int nnodes = 100;
static int perf_loops = 1000;
static int check_loops = 100;

struct test_node {
	uint32_t key;
	struct rb_node rb;

	/* following fields used for testing augmented rbtree functionality */
	uint32_t val;
	uint32_t augmented;
};

static struct rb_root_cached root = RB_ROOT_CACHED;
static struct test_node *nodes = NULL;

static void *rnd = NULL; // Unused in userspace version

static void insert(struct test_node *node, struct rb_root_cached *root)
{
	struct rb_node **new = &root->rb_root.rb_node, *parent = NULL;
	uint32_t key = node->key;

	while (*new) {
		parent = *new;
		if (key < rb_entry(parent, struct test_node, rb)->key)
			new = &parent->rb_left;
		else
			new = &parent->rb_right;
	}

	rb_link_node(&node->rb, parent, new);
	rb_insert_color(&node->rb, &root->rb_root);
}

static void insert_cached(struct test_node *node, struct rb_root_cached *root)
{
	struct rb_node **new = &root->rb_root.rb_node, *parent = NULL;
	uint32_t key = node->key;
	bool leftmost = true;

	while (*new) {
		parent = *new;
		if (key < rb_entry(parent, struct test_node, rb)->key)
			new = &parent->rb_left;
		else {
			new = &parent->rb_right;
			leftmost = false;
		}
	}

	rb_link_node(&node->rb, parent, new);
	rb_insert_color_cached(&node->rb, root, leftmost);
}

static inline void erase(struct test_node *node, struct rb_root_cached *root)
{
	rb_erase(&node->rb, &root->rb_root);
}

static inline void erase_cached(struct test_node *node, struct rb_root_cached *root)
{
	rb_erase_cached(&node->rb, root);
}


#define NODE_VAL(node) ((node)->val)

RB_DECLARE_CALLBACKS_MAX(static, augment_callbacks,
			 struct test_node, rb, uint32_t, augmented, NODE_VAL)

static void insert_augmented(struct test_node *node,
			     struct rb_root_cached *root)
{
	struct rb_node **new = &root->rb_root.rb_node, *rb_parent = NULL;
	uint32_t key = node->key;
	uint32_t val = node->val;
	struct test_node *parent;

	while (*new) {
		rb_parent = *new;
		parent = rb_entry(rb_parent, struct test_node, rb);
		if (parent->augmented < val)
			parent->augmented = val;
		if (key < parent->key)
			new = &parent->rb.rb_left;
		else
			new = &parent->rb.rb_right;
	}

	node->augmented = val;
	rb_link_node(&node->rb, rb_parent, new);
	rb_insert_augmented(&node->rb, &root->rb_root, &augment_callbacks);
}

static void insert_augmented_cached(struct test_node *node,
				    struct rb_root_cached *root)
{
	struct rb_node **new = &root->rb_root.rb_node, *rb_parent = NULL;
	uint32_t key = node->key;
	uint32_t val = node->val;
	struct test_node *parent;
	bool leftmost = true;

	while (*new) {
		rb_parent = *new;
		parent = rb_entry(rb_parent, struct test_node, rb);
		if (parent->augmented < val)
			parent->augmented = val;
		if (key < parent->key)
			new = &parent->rb.rb_left;
		else {
			new = &parent->rb.rb_right;
			leftmost = false;
		}
	}

	node->augmented = val;
	rb_link_node(&node->rb, rb_parent, new);
	rb_insert_augmented_cached(&node->rb, root,
				   leftmost, &augment_callbacks);
}


static void erase_augmented(struct test_node *node, struct rb_root_cached *root)
{
	rb_erase_augmented(&node->rb, &root->rb_root, &augment_callbacks);
}

static void erase_augmented_cached(struct test_node *node,
				   struct rb_root_cached *root)
{
	rb_erase_augmented_cached(&node->rb, root, &augment_callbacks);
}

static void init(void)
{
	int i;
	for (i = 0; i < nnodes; i++) {
		nodes[i].key = prandom_u32_state(&rnd);
		nodes[i].val = prandom_u32_state(&rnd);
	}
}

static bool is_red(struct rb_node *rb)
{
	return !(rb->__rb_parent_color & 1);
}

static int black_path_count(struct rb_node *rb)
{
	int count;
	for (count = 0; rb; rb = rb_parent(rb))
		count += !is_red(rb);
	return count;
}

static void check_postorder_foreach(int nr_nodes)
{
	struct test_node *cur, *n;
	int count = 0;
	rbtree_postorder_for_each_entry_safe(cur, n, &root.rb_root, rb)
		count++;

	TEST_ASSERT_FALSE(count != nr_nodes);
}

static void check_postorder(int nr_nodes)
{
	struct rb_node *rb;
	int count = 0;
	for (rb = rb_first_postorder(&root.rb_root); rb; rb = rb_next_postorder(rb))
		count++;

	TEST_ASSERT_FALSE(count != nr_nodes);
}

static void check(int nr_nodes)
{
	struct rb_node *rb;
	int count = 0, blacks = 0;
	uint32_t prev_key = 0;

	for (rb = rb_first(&root.rb_root); rb; rb = rb_next(rb)) {
		struct test_node *node = rb_entry(rb, struct test_node, rb);
		TEST_ASSERT_FALSE(node->key < prev_key);
		TEST_ASSERT_FALSE(is_red(rb) &&
			     (!rb_parent(rb) || is_red(rb_parent(rb))));
		if (!count)
			blacks = black_path_count(rb);
		else
			TEST_ASSERT_FALSE((!rb->rb_left || !rb->rb_right) &&
				     blacks != black_path_count(rb));
		prev_key = node->key;
		count++;
	}

	TEST_ASSERT_FALSE(count != nr_nodes);
	TEST_ASSERT_FALSE(count < (1 << black_path_count(rb_last(&root.rb_root))) - 1);

	check_postorder(nr_nodes);
	check_postorder_foreach(nr_nodes);
}

static void check_augmented(int nr_nodes)
{
	struct rb_node *rb;

	check(nr_nodes);
	for (rb = rb_first(&root.rb_root); rb; rb = rb_next(rb)) {
		struct test_node *node = rb_entry(rb, struct test_node, rb);
		uint32_t subtree, max = node->val;
		if (node->rb.rb_left) {
			subtree = rb_entry(node->rb.rb_left, struct test_node,
					   rb)->augmented;
			if (max < subtree)
				max = subtree;
		}
		if (node->rb.rb_right) {
			subtree = rb_entry(node->rb.rb_right, struct test_node,
					   rb)->augmented;
			if (max < subtree)
				max = subtree;
		}
		TEST_ASSERT_FALSE(node->augmented != max);
	}
}

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}


// Timing functionality for userspace
static inline uint64_t get_cycles(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static inline uint64_t div_u64(uint64_t dividend, uint64_t divisor) {
    return dividend / divisor;
}

void rbtree_test_init(void)
{
    int i, j;
    uint64_t time1, time2, time;
    struct rb_node *node;

    // Replace kmalloc with malloc
    nodes = malloc(nnodes * sizeof(*nodes));
    if (!nodes)
        TEST_ABORT();

    // Replace kernel PRNG with standard C PRNG
    srand(3141592653UL); // Using truncated seed for compatibility

    init();
    time1 = get_cycles();
    for (i = 0; i < perf_loops; i++) {
        for (j = 0; j < nnodes; j++)
            insert(nodes + j, &root);
        for (j = 0; j < nnodes; j++)
            erase(nodes + j, &root);
    }
    time2 = get_cycles();
    time = time2 - time1;
    time = div_u64(time, perf_loops);
    printf(" -> test 1 (latency of nnodes insert+delete): %llu cycles\n",
           (unsigned long long)time);

    time1 = get_cycles();
    for (i = 0; i < perf_loops; i++) {
        for (j = 0; j < nnodes; j++)
            insert_cached(nodes + j, &root);
        for (j = 0; j < nnodes; j++)
            erase_cached(nodes + j, &root);
    }
    time2 = get_cycles();
    time = time2 - time1;
    time = div_u64(time, perf_loops);
    printf(" -> test 2 (latency of nnodes cached insert+delete): %llu cycles\n",
           (unsigned long long)time);

    for (i = 0; i < nnodes; i++)
        insert(nodes + i, &root);
    time1 = get_cycles();
    for (i = 0; i < perf_loops; i++) {
        for (node = rb_first(&root.rb_root); node; node = rb_next(node))
            ;
    }
    time2 = get_cycles();
    time = time2 - time1;
    time = div_u64(time, perf_loops);
    printf(" -> test 3 (latency of inorder traversal): %llu cycles\n",
           (unsigned long long)time);

    time1 = get_cycles();
    for (i = 0; i < perf_loops; i++)
        node = rb_first(&root.rb_root);
    time2 = get_cycles();
    time = time2 - time1;
    time = div_u64(time, perf_loops);
    printf(" -> test 4 (latency to fetch first node)\n");
    printf("        non-cached: %llu cycles\n", (unsigned long long)time);

    time1 = get_cycles();
    for (i = 0; i < perf_loops; i++)
        node = rb_first_cached(&root);
    time2 = get_cycles();
    time = time2 - time1;
    time = div_u64(time, perf_loops);
    printf("        cached: %llu cycles\n", (unsigned long long)time);

    for (i = 0; i < nnodes; i++)
        erase(nodes + i, &root);

    /* run checks */
    for (i = 0; i < check_loops; i++) {
        init();
        for (j = 0; j < nnodes; j++) {
            check(j);
            insert(nodes + j, &root);
        }
        for (j = 0; j < nnodes; j++) {
            check(nnodes - j);
            erase(nodes + j, &root);
        }
        check(0);
    }

    printf("augmented rbtree testing\n");
    init();
    time1 = get_cycles();
    for (i = 0; i < perf_loops; i++) {
        for (j = 0; j < nnodes; j++)
            insert_augmented(nodes + j, &root);
        for (j = 0; j < nnodes; j++)
            erase_augmented(nodes + j, &root);
    }
    time2 = get_cycles();
    time = time2 - time1;
    time = div_u64(time, perf_loops);
    printf(" -> test 1 (latency of nnodes insert+delete): %llu cycles\n",
           (unsigned long long)time);

    time1 = get_cycles();
    for (i = 0; i < perf_loops; i++) {
        for (j = 0; j < nnodes; j++)
            insert_augmented_cached(nodes + j, &root);
        for (j = 0; j < nnodes; j++)
            erase_augmented_cached(nodes + j, &root);
    }
    time2 = get_cycles();
    time = time2 - time1;
    time = div_u64(time, perf_loops);
    printf(" -> test 2 (latency of nnodes cached insert+delete): %llu cycles\n",
           (unsigned long long)time);

    for (i = 0; i < check_loops; i++) {
        init();
        for (j = 0; j < nnodes; j++) {
            check_augmented(j);
            insert_augmented(nodes + j, &root);
        }
        for (j = 0; j < nnodes; j++) {
            check_augmented(nnodes - j);
            erase_augmented(nodes + j, &root);
        }
        check_augmented(0);
    }

    free(nodes);
}

void test_rbtree_functionality(void)
{
	int i, j;

	nodes = calloc(nnodes, sizeof(*nodes));
	TEST_ASSERT_NOT_NULL(nodes);

	/* run basic functionality checks */
	for (i = 0; i < check_loops; i++) {
		init();
		for (j = 0; j < nnodes; j++) {
			check(j);
			insert(nodes + j, &root);
		}
		for (j = 0; j < nnodes; j++) {
			check(nnodes - j);
			erase(nodes + j, &root);
		}
		check(0);
	}

	/* run augmented tree checks */
	for (i = 0; i < check_loops; i++) {
		init();
		for (j = 0; j < nnodes; j++) {
			check_augmented(j);
			insert_augmented(nodes + j, &root);
		}
		for (j = 0; j < nnodes; j++) {
			check_augmented(nnodes - j);
			erase_augmented(nodes + j, &root);
		}
		check_augmented(0);
	}

	free(nodes);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_rbtree_functionality);
    RUN_TEST(rbtree_test_init);
    return UNITY_END();
}
