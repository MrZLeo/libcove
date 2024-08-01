// RedBlack tree implementation
//
// properties:
// 1. Every node is either red or black
// 2. The root is black
// 3. Every leaf (NIL) is black
// 4. If a red node has children, then the children are black
// 5. Every path from a node to its descendant NIL nodes contains the same
// number of black nodes

#include "rbtree.h"
#include <stdio.h>

[[gnu::always_inline]]
static inline struct rb_node *rb_red_parent(struct rb_node *red) {
    return (struct rb_node *) red->__rb_parent_color;
}

[[gnu::always_inline]]
static inline void __rb_change_child(
    struct rb_node *old,
    struct rb_node *new,
    struct rb_node *parent,
    struct rb_root *root
) {
    if (!parent) {
        root->rb_node = new;
        return;
    }

    if (parent->rb_left == old)
        parent->rb_left = new;
    else
        parent->rb_right = new;
}

[[gnu::always_inline]]
static inline void __rb_rotate_left(
    struct rb_node *parent,
    struct rb_node *child,
    uint64_t color
) {
    struct rb_node *cl = child->rb_left;
    parent->rb_right = cl;
    child->rb_left = parent;
    if (cl)
        rb_set_parent_color(cl, parent, color);
}

[[gnu::always_inline]]
static inline void __rb_rotate_right(
    struct rb_node *parent,
    struct rb_node *child,
    uint64_t color
) {
    struct rb_node *cr = child->rb_left;
    parent->rb_left = cr;
    child->rb_right = parent;
    if (cr)
        rb_set_parent_color(cr, parent, color);
}

[[gnu::always_inline]]
static inline void __rb_rotate_set_parent(
    struct rb_node *old,
    struct rb_node *new,
    struct rb_root *root,
    uint64_t color
) {
    struct rb_node *parent = rb_parent(old);
    new->__rb_parent_color = old->__rb_parent_color;
    rb_set_parent_color(old, new, color);
    __rb_change_child(old, new, parent, root);
}

[[gnu::always_inline]]
static inline void __rb_insert(
    struct rb_root *root,
    struct rb_node *node,
    void (*augment_rotate)(struct rb_node *old, struct rb_node *new)
) {
    struct rb_node *parent = rb_red_parent(node);
    struct rb_node *gparent;
    struct rb_node *tmp;

    // Loop invariant: node is red
    while (true) {
        if (!parent) {
            // node is the root
            rb_set_parent_color(node, NULL, RB_BLACK);
            break;
        }

        // everything is fine, we are done
        if (rb_is_black(parent))
            break;

        // parent is red, so grandparent must exist and must be balck
        // next we concern about the uncle
        gparent = rb_red_parent(parent);

        tmp = gparent->rb_right;
        if (tmp != parent) {  // parent is the left child
            if (tmp && rb_is_red(tmp)) {
                /*
                 * case 1: uncle is red
                 *          gparent(B)
                 *          /       \
                 *      parent(R)   uncle(R)
                 *      /
                 *  node(R)
                 */

                // 1. flip the color of parent and uncle
                rb_set_parent_color(parent, gparent, RB_BLACK);
                rb_set_parent_color(tmp, gparent, RB_BLACK);

                // 2. flip the color of gparent
                node = gparent;
                parent = rb_parent(node);
                rb_set_parent_color(node, parent, RB_RED);
                continue;
            }

            tmp = parent->rb_right;
            if (tmp == node) {
                /*
                 * case 3(LR Inbalence):
                 * uncle is black or NIL, and node is the right child
                 *
                 *         gparent(B)                     gparent(B)
                 *         /       \                      /     \
                 *     parent(R)  uncle(B)/NIL  ->  node(R)  uncle(B)/NIL
                 *     /     \                      /
                 *   tmp(B)  node(R)            parent(R)
                 *            /                  /  \
                 *      rb_left(B)          tmp(B)  rb_left(B)
                 */

                // 1. rotate left at parent
                tmp = node->rb_left;
                parent->rb_right = tmp;
                node->rb_left = parent;
                if (tmp)
                    rb_set_parent_color(tmp, parent, RB_BLACK);
                rb_set_parent_color(parent, node, RB_RED);
                augment_rotate(parent, node);
                parent = node;
                tmp = parent->rb_right;
            }

            /*
             * case 2(LL Inbalence):
             * uncle is black or NIL, and node is the left child
             *
             *          gparent(B)                     parent(B)
             *          /       \                      /       \
             *      parent(R)  uncle(B)/NIL  ->  node(R)   gparent(R)
             *      /     \                                  /     \
             *  node(R)  tmp(B)                        tmp(B)  uncle(B)/NIL
             */

            // 1. rotate right at parent
            gparent->rb_left = tmp;
            parent->rb_right = gparent;
            if (tmp)
                rb_set_parent_color(tmp, gparent, RB_BLACK);

            // 2. flip the color of parent and gparent
            __rb_rotate_set_parent(gparent, parent, root, RB_RED);
            augment_rotate(gparent, parent);
            break;
        } else {  // parent is the right child
            tmp = gparent->rb_left;
            if (tmp && rb_is_red(tmp)) {
                /*
                 * case 1: uncle is red
                 *          gparent(B)
                 *          /       \
                 *      uncle(R)   parent(R)
                 *                      \
                 *                      node(R)
                 */

                // 1. flip the color of parent and uncle
                rb_set_parent_color(parent, gparent, RB_BLACK);
                rb_set_parent_color(tmp, gparent, RB_BLACK);

                // 2. flip the color of gparent
                node = gparent;
                parent = rb_parent(node);
                rb_set_parent_color(node, parent, RB_RED);
                continue;
            }

            tmp = parent->rb_left;
            if (tmp == node) {
                /*
                 * case 3(RL Inbalence):
                 * uncle is black or NIL, and node is the left child
                 *         gparent(B)                   gparent(B)
                 *         /       \                    /       \
                 *  uncle(B)/NIL  parent(R)  ->  uncle(B)/NIL node(R)
                 *                  /     \                       \
                 *              node(R)  tmp(B)                 parent(R)
                 *                  \                            /  \
                 *                 rb_right(B)          rb_right(B) tmp(B)
                 */

                // 1. rotate right at parent
                tmp = node->rb_right;
                parent->rb_left = tmp;
                node->rb_right = parent;
                if (tmp)
                    rb_set_parent_color(tmp, parent, RB_BLACK);

                rb_set_parent_color(parent, node, RB_RED);
                augment_rotate(parent, node);
                parent = node;
                tmp = parent->rb_left;
            }

            /*
             * case 4(RR Inbalence):
             * uncle is black or NIL, and node is the right child
             *
             *         gparent(B)                   parent(B)
             *          /       \                   /       \
             *      uncle(B)  parent(R)  ->     gparent(R)  node(R)
             *                 /    \             /     \
             *              tmp(B)  node(R)   uncle(B)  tmp(B)
             */

            // 1. rotate left at gparent
            tmp = parent->rb_left;
            gparent->rb_right = tmp;
            parent->rb_left = gparent;
            if (tmp)
                rb_set_parent_color(tmp, gparent, RB_BLACK);

            // 2. flip the color of parent and gparent
            __rb_rotate_set_parent(gparent, parent, root, RB_RED);
            augment_rotate(gparent, parent);
            break;
        }
    }
}

void(augment_rotate)(
    [[maybe_unused]] struct rb_node *old,
    [[maybe_unused]] struct rb_node *new
) {
#ifdef __LIBCOVE_RBTREE_DEBUG__
    printf("rotate: %p -> %p\n", old, new);
#endif
}

void rb_insert_color(struct rb_root *root, struct rb_node *node) {
    __rb_insert(root, node, augment_rotate);
}

[[gnu::always_inline]]
static inline struct rb_node *__rb_erase(
    struct rb_root *root,
    struct rb_node *node
) {
    struct rb_node *tmp = node->rb_left;
    struct rb_node *child = node->rb_right;
    struct rb_node *parent = rb_parent(node);

    if (!tmp) {
        // case1: node has only one child, or no child
        //
        // If only one child, then we can make sure that:
        // **node is BLACK and child is RED.**
        //
        // Because properties of 5 tells us that if child
        // is black, then the path from node to NIL has different number of
        // black nodes due to `!tmp`.
        __rb_change_child(node, child, parent, root);

        struct rb_node prev_node = *node;
        if (child) {
            child->__rb_parent_color = node->__rb_parent_color;
            return NULL;
        }
        return rb_is_black(&prev_node) ? parent : NULL;
    } else if (!child) {
        // The same as case1, and tmp is the only child
        __rb_change_child(node, tmp, parent, root);
        tmp->__rb_parent_color = node->__rb_parent_color;
        return NULL;
    } else {
        // case2: node has two children
        // We need to find the successor of node, and replace node with it
        //
        // invariants:
        //  1. tmp is node->left
        //  2. child is node->right
        //  3. successor is the leftmost child of child
        //  4. succ_right is successor->right
        //  5. parent is the parent of succ_right
        struct rb_node *successor = child;
        struct rb_node *succ_right;
        if (!successor->rb_left) {
            // child has no left child, then child is the successor
            succ_right = successor->rb_right;
            parent = successor;
        } else {
            // child has left child, then we need to find the successor
            // of node in the right subtree of child
            do {
                parent = successor;
                successor = successor->rb_left;
            } while (successor->rb_left);
            succ_right = successor->rb_right;
            parent->rb_left = succ_right;
            successor->rb_right = child;
            rb_set_parent(child, successor);
        }

        successor->rb_left = tmp;
        rb_set_parent(tmp, successor);

        struct rb_node prev_succ = *successor;
        successor->__rb_parent_color = node->__rb_parent_color;
        /*rb_set_parent(successor, rb_parent(node));*/
        __rb_change_child(node, successor, rb_parent(node), root);

        // we move successor to node, making nodes in path through successor -1,
        // so we need to check whether the BLACK node is missing
        //
        // 1. if successor is RED, then succ_right must be BLACK.
        // 2. if successor is BLACK, we lost 1 BLACK, so make succ_right to be
        // black is enough
        //
        // accroding to 2 cases, we can just setting succ_right to be BLACK.
        // However, it succ_right doesn't exist, we need to recover from parent
        // if successor is BLACK.
        if (succ_right) {
            rb_set_parent_color(succ_right, parent, RB_BLACK);
            return NULL;
        }
        return rb_is_black(&prev_succ) ? parent : NULL;
    }
}

[[gnu::always_inline, gnu::nonnull(1, 2)]]
static inline void __rb_erase_rebalance(
    struct rb_root *root,
    struct rb_node *parent
) {
    struct rb_node *node = NULL;
    struct rb_node *siblings;
    struct rb_node *tmp1, *tmp2;

    // Loop invariants:
    //  1. node is BLACK(or NIL in first iteration)
    //  2. node is not root
    //  3. all leaf paths going through parent and node have 1 BLACK node
    // that is lower than other path
    while (true) {
        siblings = parent->rb_right;
        if (siblings != node) {
            if (rb_is_red(siblings)) {
                /* case 1: siblings is RED
                 * parent and children of siblings must be BLACK.
                 *
                 *         parent(B)                     siblings(B)
                 *        /       \                      /       \
                 *    node(B)  siblings(R)  ->      parent(R)   r(B)
                 *              /   \               /     \
                 *          l(B)   r(B)         node(B)  l(B)
                 *
                 * Rotate left at parent, flip the color of siblings and parent.
                 * Then we enter case5, which can be maintained by next loop.
                 */
                tmp1 = siblings->rb_left;
                __rb_rotate_left(parent, siblings, RB_BLACK);
                __rb_rotate_set_parent(parent, siblings, root, RB_RED);
                siblings = tmp1;
            }
            // continue with root as parent
            tmp1 = siblings->rb_right;
            if (!tmp1 || rb_is_black(tmp1)) {
                tmp2 = siblings->rb_left;
                if (!tmp2 || rb_is_black(tmp2)) {
                    /*
                     * case2: siblings is BLACK, and both children are BLACK
                     *
                     *         parent(R)                     parent(B)
                     *        /       \                      /       \
                     *    node(B)  siblings(B)  ->      node(R)   siblings(R)
                     *              /   \                           /   \
                     *          l(B)   r(B)                     l(B)    r(B)
                     *
                     */
                    rb_set_parent_color(siblings, parent, RB_RED);
                    if (rb_is_red(parent)) {
                        rb_set_black(parent);
                        break;
                    }
                    // or we enter case 3
                    /*
                     * case3:
                     * parent and siblings is BLACK, and both children are BLACK
                     *
                     *         parent(B)                    parent(B)
                     *          /   \                       /       \
                     *    node(B)  siblings(B)  ->      node(R)   siblings(R)
                     *              /   \                           /   \
                     *          l(B)   r(B)                     l(B)    r(B)
                     *
                     */
                    node = parent;
                    parent = rb_parent(node);
                    continue;
                }
                /*
                 * case 4:
                 * siblings is BLACK, left child is RED, right child is BLACK
                 *
                 *         parent(R/B)                  parent(B)
                 *          /   \                       /   \
                 *    node(B)  siblings(B)  ->      node(B) tmp2(B)
                 *              /   \                           \
                 *          tmp2(R) tmp1(B)                 siblings(R)
                 *              \                            /      \
                 *              tmp2_r                  tmp2_r    tmp1(B)
                 *
                 * right rotate at siblings, flip color of siblings and left
                 * child, then we enter case 5
                 */
                __rb_rotate_right(siblings, tmp2, RB_BLACK);
                __rb_rotate_set_parent(siblings, tmp2, root, RB_RED);

                siblings = tmp2;
                tmp1 = siblings->rb_right;
                tmp2 = siblings->rb_left;
            }
            /*
             * case5: siblings is BLACK, right child is RED
             *
             *         parent(R/B)                  siblings(R/B)
             *          /   \                       /       \
             *    node(B)  siblings(B)  ->      parent(B)   tmp1(B)
             *               /  \                /    \
             *           tmp2   tmp1(R)       node(B) tmp2
             *
             *  left rotate at parent, flip color of parent and siblings,
             *  then make tmp1 to be BLACK
             */
            __rb_rotate_left(parent, siblings, RB_BLACK);
            __rb_rotate_set_parent(parent, siblings, root, RB_BLACK);
        } else {
            // FIXME
            siblings = parent->rb_left;
            if (rb_is_red(siblings)) {
                // case1
                tmp1 = siblings->rb_right;
                __rb_rotate_right(parent, siblings, RB_BLACK);
                __rb_rotate_set_parent(parent, siblings, root, RB_RED);
                siblings = tmp1;
            }
            tmp1 = siblings->rb_left;
            if (!tmp1 || rb_is_black(tmp1)) {
                tmp2 = siblings->rb_right;
                if (!tmp2 || rb_is_black(tmp2)) {
                    // case2
                    rb_set_parent_color(siblings, parent, RB_RED);
                    if (rb_is_red(parent)) {
                        rb_set_black(parent);
                        break;
                    }
                    // case3
                    node = parent;
                    parent = rb_parent(node);
                    continue;
                }
                // case4
                __rb_rotate_left(siblings, tmp2, RB_BLACK);
                __rb_rotate_set_parent(siblings, tmp2, root, RB_RED);

                siblings = tmp2;
                tmp1 = siblings->rb_left;
                tmp2 = siblings->rb_right;
            }
            // case5
            __rb_rotate_right(parent, siblings, RB_BLACK);
            __rb_rotate_set_parent(parent, siblings, root, RB_BLACK);
        }
    }
}

void rb_erase(struct rb_root *root, struct rb_node *node) {
    struct rb_node *rebalance = __rb_erase(root, node);
    if (rebalance)
        __rb_erase_rebalance(root, rebalance);
}
