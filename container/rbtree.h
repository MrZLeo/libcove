#ifndef LIBCOVE_RBTREE_H
#define LIBCOVE_RBTREE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct rb_node {
    uintptr_t __rb_parent_color;
    struct rb_node *rb_left;
    struct rb_node *rb_right;
} __attribute__((aligned(sizeof(uintptr_t))));

struct rb_root {
    struct rb_node *rb_node;
};

typedef int (*rb_compar_fn_t)(const void *key, const struct rb_node *node);
typedef bool (*rb_less_fn_t)(const struct rb_node *l, const struct rb_node *r);

void rb_insert_color(struct rb_root *root, struct rb_node *node);
void rb_erase(struct rb_root *root, struct rb_node *node);

/* Find logical next and previous nodes in a tree */
struct rb_node *rb_next(const struct rb_node *cur);
struct rb_node *rb_prev(const struct rb_node *cur);
struct rb_node *rb_first(const struct rb_root *root);
struct rb_node *rb_last(const struct rb_root *root);

[[gnu::always_inline]]
static inline void rb_link_node(
    struct rb_node *node,
    struct rb_node *parent,
    struct rb_node **rb_link
) {
    node->__rb_parent_color = (uintptr_t) parent;
    node->rb_left = node->rb_right = NULL;
    *rb_link = node;
}

[[gnu::always_inline, gnu::nonnull(1)]]
static inline void rb_add(
    struct rb_root *root,
    struct rb_node *node,
    rb_less_fn_t less
) {
    struct rb_node **link = &root->rb_node;
    struct rb_node *parent = NULL;

    while (*link) {
        parent = *link;
        if (less(node, parent))
            link = &parent->rb_left;
        else
            link = &parent->rb_right;
    }

    rb_link_node(node, parent, link);
    rb_insert_color(root, node);
}

[[gnu::always_inline]]
static inline struct rb_node *rb_find(
    const struct rb_root *root,
    const void *key,
    rb_compar_fn_t compar
) {
    struct rb_node *cur = root->rb_node;

    while (cur) {
        int c = compar(key, cur);

        if (c < 0)
            cur = cur->rb_left;
        else if (c > 0)
            cur = cur->rb_right;
        else
            return cur;
    }

    return NULL;
}

// ---
#define RB_RED   0
#define RB_BLACK 1

#define __rb_parent(pc) ((struct rb_node *) ((pc) & ~3))
#define rb_parent(rb)   __rb_parent((rb)->__rb_parent_color)

#define __rb_color(pc)    ((pc) & 1)
#define __rb_is_black(pc) __rb_color(pc)
#define __rb_is_red(pc)   (!__rb_color(pc))

#define rb_color(rb)    (bool) __rb_color((rb)->__rb_parent_color)
#define rb_is_black(rb) (bool) __rb_is_black((rb)->__rb_parent_color)
#define rb_is_red(rb)   (bool) __rb_is_red((rb)->__rb_parent_color)

[[gnu::always_inline]]
static inline void rb_set_parent_color(
    struct rb_node *node,
    struct rb_node *parent,
    uint64_t color
) {
    node->__rb_parent_color = (uintptr_t) parent + color;
}

[[gnu::always_inline]]
static inline void rb_set_parent(struct rb_node *node, struct rb_node *parent) {
    node->__rb_parent_color = rb_color(node) + (unsigned long) parent;
}

[[gnu::always_inline]]
static inline void rb_set_black(struct rb_node *node) {
    node->__rb_parent_color += RB_BLACK;
}

#define rb_entry(ptr, type, member) container_of(ptr, type, member)

#endif  // LIBCOVE_RBTREE_H
