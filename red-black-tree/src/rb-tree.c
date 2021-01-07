/**
 * @file rb-tree.c
 * @author BlaCkinkGJ (ss5kijun@gmail.com)
 * @brief red black tree implementation
 * @version 0.1
 * @date 2020-05-29
 * 
 * @ref Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009). Introduction to algorithms. MIT press.
 * @copyright Copyright (c) 2020 BlaCkinkGJ
 * 
 */
#include <stdlib.h>
#include "rb-tree.h"

static struct rb_global_info rb_info = {
        .nil = { 
                .color = RB_NODE_COLOR_BLACK,

                .key = RB_NODE_NIL_KEY_VALUE,
                .data = NULL,

                .left = NULL,
                .right = NULL,
                .parent = NULL,
        },
}; /**< global red-black information */

/**
 * @brief Allocation of red-black tree
 * 
 * @return struct rb_tree* allocated red-black tree
 */
struct rb_tree *rb_tree_alloc(void)
{
        struct rb_tree *tree = (struct rb_tree *)malloc(sizeof(struct rb_tree));
        if (!tree) {
                pr_info("Memory shortage detected! Allocation failed...");
                goto exception;
        }

        tree->nil = &rb_info.nil;
        tree->root = tree->nil;
        tree->bh = 0;

        return tree;
exception:
        if (tree->nil) {
                free(tree->nil);
                tree->nil = NULL;
        }

        if (tree) {
                free(tree);
        }
        return NULL;
}

/**
 * @brief Red-black tree left rotation
 *     (x)                    (y)
 *    ／  ＼                 ／  ＼
 *   α     (y)     ==>     (x)    γ
 *        ／  ＼          ／  ＼
 *        β    γ         α     β
 * 
 * @param tree red-black tree structure
 * @param x subtree's root node
 */
static void rb_tree_left_rotate(struct rb_tree *tree, struct rb_node *x)
{
        struct rb_node *y = NULL;

        y = x->right; /**< set right node */

        x->right = y->left; /**< move subtree */
        if (y->left != tree->nil) {
                y->left->parent = x;
        }
        y->parent = x->parent; /**< change parents */

        if (x->parent == tree->nil) {
                tree->root = y;
        } else if (x == x->parent->left) {
                x->parent->left = y;
        } else {
                x->parent->right = y;
        }

        y->left = x;
        x->parent = y;
}

/**
 * @brief Red-black tree left rotation
 *      (y)              (x)
 *     ／  ＼           ／  ＼
 *   (x)     γ  ==>    α     (y)
 *  ／  ＼                  ／  ＼
 * α     β                 β     γ
 * 
 * @param tree red-black tree structure
 * @param x subtree's root node
 */
static void rb_tree_right_rotate(struct rb_tree *tree, struct rb_node *y)
{
        struct rb_node *x = NULL;

        x = y->left; /**< set left node */

        y->left = x->right; /**< move subtree */
        if (x->right != tree->nil) {
                x->right->parent = y;
        }
        x->parent = y->parent; /**< change parents */

        if (y->parent == tree->nil) {
                tree->root = x;
        } else if (y == y->parent->right) {
                y->parent->right = x;
        } else {
                y->parent->left = x;
        }

        x->right = y;
        y->parent = x;
}

/**
 * @brief Search red-black tree's node which the same value of key.
 * Based on binary search method
 * 
 * @param root red-black tree's root
 * @param key the key which I want to search
 * @return struct rb_node* if find success then return specific node pointer.
 * but if find failed then return NULL pointer
 * d
 * @ref Horowitz, E., Sahni, S., & Anderson-Freed, S. (1992). Fundamentals of data structures in C. WH Freeman & Co..
 */
static struct rb_node *__rb_tree_search(struct rb_node *root, key_t key)
{
        struct rb_node *node = root;
        while (node) {
                if (key == node->key) { /**< Find the specific values */
                        break;
                }

                if (key < node->key) {
                        node = node->left;
                } else {
                        node = node->right;
                }
        }

        return node;
}

/**
 * @brief Wrapping function of `__rb_tree_search` function
 * 
 * @param tree red-black tree whole
 * @param key key which I want to find
 * @return struct rb_node* if find success then return specific node pointer.
 * but if find failed then return NULL pointer
 */
struct rb_node *rb_tree_search(struct rb_tree *tree, key_t key)
{
        return __rb_tree_search(tree->root, key);
}

/**
 * @brief Get bh(black-height) of red-black tree's node which the same value of key.
 * Based on binary search method
 * 
 * @param root red-black tree's root
 * @param key the key which I want to get bh
 * @return struct rb_node* specific node's bh
 * d
 * @ref Horowitz, E., Sahni, S., & Anderson-Freed, S. (1992). Fundamentals of data structures in C. WH Freeman & Co..
 */
size_t __rb_tree_get_bh(struct rb_tree *tree, struct rb_node *root, key_t key)
{
        size_t bh = tree->bh;
        struct rb_node *node = root;
        while (node) {
                if (key == node->key) { /**< Find the specific values */
                        break;
                }

                if (node->color == RB_NODE_COLOR_BLACK) {
                        bh -= 1;
                }
                if (key < node->key) {
                        node = node->left;
                } else {
                        node = node->right;
                }
        }

        if (node == NULL) {
                bh = RB_INVALID_BLACK_HEIGHT;
        }

        return bh;
}

/**
 * @brief Wrapping function of `__rb_tree_get_bh` function
 * 
 * @param tree red-black tree whole
 * @param key specific key which I want to get black-height
 * @return size_t size of black height
 */
size_t rb_tree_get_bh(struct rb_tree *tree, key_t key)
{
        return __rb_tree_get_bh(tree, tree->root, key);
}

/**
 * @brief Re-color nodes and perform rotations
 * @details
 * case 1: z's uncle is y is red
 * case 2: z's uncle is y is black and z is a right child
 * case 3: z's uncle y is black and z is a left child
 * 
 * @param tree red-black tree structure
 * @param z new node which insert into red-black tree
 */
static void rb_tree_insert_fixup(struct rb_tree *tree, struct rb_node *z)
{
        struct rb_node *y = NULL;

        while (z->parent->color == RB_NODE_COLOR_RED) {
                if (z->parent == z->parent->parent->left) {
                        y = z->parent->parent->right;
                        if (y->color == RB_NODE_COLOR_RED) { /**< case 1 */
                                z->parent->color = RB_NODE_COLOR_BLACK;
                                y->color = RB_NODE_COLOR_BLACK;
                                z->parent->parent->color = RB_NODE_COLOR_RED;
                                z = z->parent->parent;
                        } else {
                                if (z == z->parent->right) { /**< case 2 */
                                        z = z->parent;
                                        rb_tree_left_rotate(tree, z);
                                }
                                z->parent->color =
                                        RB_NODE_COLOR_BLACK; /**< case 3 */
                                z->parent->parent->color = RB_NODE_COLOR_RED;
                                rb_tree_right_rotate(tree, z->parent->parent);
                        }
                } else { /**< only different part is left and right */
                        y = z->parent->parent->left;
                        if (y->color == RB_NODE_COLOR_RED) {
                                z->parent->color = RB_NODE_COLOR_BLACK;
                                y->color = RB_NODE_COLOR_BLACK;
                                z->parent->parent->color = RB_NODE_COLOR_RED;
                                z = z->parent->parent;
                        } else {
                                if (z == z->parent->left) {
                                        z = z->parent;
                                        rb_tree_right_rotate(tree, z);
                                }
                                z->parent->color = RB_NODE_COLOR_BLACK;
                                z->parent->parent->color = RB_NODE_COLOR_RED;
                                rb_tree_left_rotate(tree, z->parent->parent);
                        }
                }
        }

        if (tree->root->color == RB_NODE_COLOR_RED) {
                tree->bh += 1;
        }
        tree->root->color = RB_NODE_COLOR_BLACK;
}

/**
 * @brief Insert node to red-black tree
 * 
 * @param tree red-black tree structure
 * @param z new node which insert into red-black tree
 * @return int successfully insert status (0: success, else: fail)
 */
static int __rb_tree_insert(struct rb_tree *tree, struct rb_node *z)
{
        struct rb_node *y = NULL;
        struct rb_node *x = NULL;

        if (z == NULL) {
                pr_info("z must be not null");
                return -ENOMEM;
        }

        if (z->key == RB_NODE_NIL_KEY_VALUE) {
                pr_info("%ld key value is preserved by tree->nil\n",
                        RB_NODE_NIL_KEY_VALUE);
                return -EINVAL;
        }

        y = tree->nil;
        x = tree->root;
        while (x != tree->nil) {
                if (x->key == z->key) { /**< update key's data */
                        rb_node_move(x, z);
                        return 0;
                }
                y = x;
                if (z->key < x->key) {
                        x = x->left;
                } else {
                        x = x->right;
                }
        } /**< traverse valid insert location */

        z->parent = y;
        if (y == tree->nil) { /**< set y state */
                tree->root = z;
        } else if (z->key < y->key) {
                y->left = z;
        } else {
                y->right = z;
        }

        if (z->left == NULL) {
                z->left = tree->nil;
        }
        if (z->right == NULL) {
                z->right = tree->nil;
        }
        z->color = RB_NODE_COLOR_RED;

        rb_tree_insert_fixup(tree, z);

        return 0;
}

/**
 * @brief Wrapping function of `__rb_tree_insert`
 * 
 * @param tree red-black tree structure
 * @param key new node's key
 * @param data new node's data
 * @return int successfully insert status (0: success, else: fail)
 */
int rb_tree_insert(struct rb_tree *tree, const key_t key, void *data)
{
        struct rb_node *node = NULL;
        int ret;

        node = rb_node_alloc(key);
        if (!node) {
                pr_info("Allocate the node failed");
                return -ENOMEM;
        }

        node->data = data;

        ret = __rb_tree_insert(tree, node);
        if (ret == -EINVAL) {
                rb_node_dealloc(node);
        }

        return ret;
}

/**
 * @brief Translant previous root to next root
 * 
 * @param tree red-black tree whole
 * @param prev_root previous root node
 * @param next_root next root node which want to transplant
 */
static void rb_tree_transplant(struct rb_tree *tree, struct rb_node *prev_root,
                               struct rb_node *next_root)
{
        if (prev_root->parent == tree->nil) {
                tree->root = next_root;
        } else if (prev_root == prev_root->parent->left) {
                prev_root->parent->left = next_root;
        } else {
                prev_root->parent->right = next_root;
        }

        next_root->parent = prev_root->parent;
}

/**
 * @brief Get red-black tree's minimum node
 * 
 * @param tree red-black tree whole
 * @param root root of red-black tree
 * @return struct rb_node* minimum red-black tree node
 */
struct rb_node *rb_tree_minimum(struct rb_tree *tree, struct rb_node *root)
{
        if (root == tree->nil) {
                return root;
        }

        while (root->left != tree->nil) {
                root = root->left;
        }

        return root;
}

/**
 * @brief Get red-black tree's maximum node
 * 
 * @param tree red-black tree whole
 * @param root root of red-black tree
 * @return struct rb_node* maximum red-black tree node
 */
struct rb_node *rb_tree_maximum(struct rb_tree *tree, struct rb_node *root)
{
        if (root == tree->nil) {
                return root;
        }

        while (root->right != tree->nil) {
                root = root->right;
        }

        return root;
}

/**
 * @brief successor of specific node location
 * 
 * @param tree red-black tree whole
 * @param x specific node location of tree
 * @return struct rb_node* node which successor of specific node location
 */
struct rb_node *rb_tree_successor(struct rb_tree *tree, struct rb_node *x)
{
        struct rb_node *y = NULL;

        if (x->right != tree->nil) {
                return rb_tree_minimum(tree, x->right);
        }

        y = x->parent;

        while (y != tree->nil && x == y->right) {
                x = y;
                y = y->parent;
        }

        return y;
}

/**
 * @brief predecessor of specific node location
 * 
 * @param tree red-black tree whole
 * @param x specific node location of tree
 * @return struct rb_node* node which predecessor of specific node location
 */
struct rb_node *rb_tree_predecessor(struct rb_tree *tree, struct rb_node *y)
{
        struct rb_node *x = NULL;

        if (y->left != tree->nil) {
                return rb_tree_maximum(tree, y->left);
        }

        x = y->parent;

        while (x != tree->nil && y == x->left) {
                y = x;
                x = x->parent;
        }

        return x;
}
/**
 * @brief Re-color nodes and perform rotations
 * @details
 * case 1: x's sibling w is red
 * case 2: x's sibling w is black, and both of w's children are black 
 * case 3: x's sibling w is black, w's left child is red, and w's right child is black
 * case 4: x's sibling w is black, and w's right child is red
 * 
 * @param tree red-black tree structure
 * @param x rotation key node pointer
 */
static void rb_tree_delete_fixup(struct rb_tree *tree, struct rb_node *x)
{
        struct rb_node *w = NULL;
        int is_forced = 0;
        int is_goes_up = 0;

        while (x != tree->root && x->color == RB_NODE_COLOR_BLACK) {
                is_goes_up = 1;
                if (x == x->parent->left) {
                        w = x->parent->right;
                        if (w->color == RB_NODE_COLOR_RED) {
                                w->color = RB_NODE_COLOR_BLACK;
                                x->parent->color = RB_NODE_COLOR_RED;
                                rb_tree_left_rotate(tree, x->parent);
                                w = x->parent->right;
                        } /**< case 1 */

                        if (w->left->color == RB_NODE_COLOR_BLACK &&
                            w->right->color == RB_NODE_COLOR_BLACK) {
                                w->color = RB_NODE_COLOR_RED;
                                x = x->parent;
                        } /**< case 2 */
                        else {
                                if (w->right->color == RB_NODE_COLOR_BLACK) {
                                        w->left->color = RB_NODE_COLOR_BLACK;
                                        w->color = RB_NODE_COLOR_RED;
                                        rb_tree_right_rotate(tree, w);
                                        w = x->parent->right;
                                } /**< case 3 */

                                w->color = x->parent->color;
                                x->parent->color = RB_NODE_COLOR_BLACK;
                                w->right->color = RB_NODE_COLOR_BLACK;
                                rb_tree_left_rotate(tree, x->parent);
                                x = tree->root; /**< case 4 */
                                is_forced = 1;
                        }
                } else { /**< only different part is left and right */
                        w = x->parent->left;
                        if (w->color == RB_NODE_COLOR_RED) {
                                w->color = RB_NODE_COLOR_BLACK;
                                x->parent->color = RB_NODE_COLOR_RED;
                                rb_tree_right_rotate(tree, x->parent);
                                w = x->parent->left;
                        } /**< case 1 */

                        if (w->right->color == RB_NODE_COLOR_BLACK &&
                            w->left->color == RB_NODE_COLOR_BLACK) {
                                w->color = RB_NODE_COLOR_RED;
                                x = x->parent;
                        } /**< case 2 */
                        else {
                                if (w->left->color == RB_NODE_COLOR_BLACK) {
                                        w->right->color = RB_NODE_COLOR_BLACK;
                                        w->color = RB_NODE_COLOR_RED;
                                        rb_tree_left_rotate(tree, w);
                                        w = x->parent->left;
                                } /**< case 3 */

                                w->color = x->parent->color;
                                x->parent->color = RB_NODE_COLOR_BLACK;
                                w->left->color = RB_NODE_COLOR_BLACK;
                                rb_tree_right_rotate(tree, x->parent);
                                x = tree->root; /**< case 4 */
                                is_forced = 1;
                        }
                }
        }
        if (x == tree->nil || (is_goes_up && !is_forced && x == tree->root)) {
                tree->bh -= 1;
        }
        x->color = RB_NODE_COLOR_BLACK;
}

/**
 * @brief delete specific node's in red-black tree
 * 
 * @param tree red-black tree whole
 * @param z delete target node
 */
static void __rb_tree_delete(struct rb_tree *tree, struct rb_node *z)
{
        struct rb_node *x = NULL;
        struct rb_node *y = NULL;

        enum rb_node_color y_original_color;

        y = z;
        y_original_color = y->color;
        if (z->left == tree->nil) {
                x = z->right;
                rb_tree_transplant(tree, z, z->right);
        } else if (z->right == tree->nil) {
                x = z->left;
                rb_tree_transplant(tree, z, z->left);
        } else {
                y = rb_tree_minimum(tree, z->right);
                y_original_color = y->color;
                x = y->right;
                if (y->parent == z) {
                        x->parent = y;
                } else {
                        rb_tree_transplant(tree, y, y->right);
                        y->right = z->right;
                        y->right->parent = y;
                }
                rb_tree_transplant(tree, z, y);
                y->left = z->left;
                y->left->parent = y;
                y->color = z->color;
        }

        if (y_original_color == RB_NODE_COLOR_BLACK) {
                rb_tree_delete_fixup(tree, x);
        }
}

/**
 * @brief Wrapping function of `__rb_tree_delete`
 * 
 * @param tree red-black tree whole
 * @param key delete target node's key
 * @return int 0 means that delete success. Not 0 means delete fail.
 */
int rb_tree_delete(struct rb_tree *tree, key_t key)
{
        struct rb_node *node = rb_tree_search(tree, key);
        if (!node) {
                return -ENODATA;
        }
        __rb_tree_delete(tree, node);
        rb_node_dealloc(node);
        return 0;
}

/**
 * @brief Concatenate two red-black tree by using node x
 * 
 * @param t1 red-black tree which have all value is smaller than x->key
 * @param t2 red-black tree which have all value is greater than x->key
 * @param x node which value is over max(t1->key) < x < min(t2->key)
 * @return struct rb_tree* 
 * 
 * @ref Introduction to Algorithms(CLRS) ▶ red-black tree chapter ▶ problem 13-2
 */
struct rb_tree *rb_tree_concat(struct rb_tree *t1, struct rb_tree *t2,
                               struct rb_node *x)
{
        struct rb_tree *new_tree = NULL;
        struct rb_node *y = NULL;
        struct rb_node *x1_max_node = NULL;
        struct rb_node *x2_min_node = NULL;

        key_t x1_max_key = RB_NODE_NIL_KEY_VALUE;
        key_t x2_min_key = RB_NODE_NIL_KEY_VALUE;

        size_t bh = 0;

        x1_max_node = rb_tree_maximum(t1, t1->root);
        x2_min_node = rb_tree_minimum(t2, t2->root);

        x1_max_key = x1_max_node->key;
        x2_min_key = x2_min_node->key;

        /**< Originally allow the same key. But this version doesn't allow it */
        if (!(x1_max_key <= x->key && x->key <= x2_min_key)) {
                pr_info("invalid state key state x1.key(%ld) <= x.key(%ld) <= x2.key(%ld)\n",
                        x1_max_key, x->key, x2_min_key);
                return NULL;
        }

        if (x->key == x1_max_key || x->key == x2_min_key) {
                struct rb_node *prev_x = x;
                struct rb_tree *tree = (x->key == x1_max_key ? t1 : t2);
                __rb_tree_delete(tree, prev_x);
                x = rb_node_alloc(prev_x->key);
                x->data = prev_x->data;
                prev_x->data = NULL;
                rb_node_dealloc(prev_x);
        }

        new_tree = rb_tree_alloc();
        if (!new_tree) {
                pr_info("new tree allocation failed...\n");
                return NULL;
        }

        if (t1->bh >= t2->bh) {
                y = t1->root;
                bh = t1->bh;
                while (bh != t2->bh) {
                        if (y->right != t1->nil) {
                                y = y->right;
                        } else if (y->left != t1->nil) {
                                y = y->left;
                        } else {
                                break;
                        }

                        if (y->color == RB_NODE_COLOR_BLACK) {
                                bh -= 1;
                        }
                }

                rb_tree_transplant(t1, y, x);
                x->left = y;
                x->right = t2->root;
                y->parent = x;
                t2->root->parent = x;

                rb_tree_insert_fixup(t1, x);

                rb_tree_copy(new_tree, t1);
        } else { /**> symmetric of previous sequence */
                y = t2->root;
                bh = t2->bh;
                while (bh != t1->bh) {
                        if (y->left != t2->nil) {
                                y = y->left;
                        } else if (y->right != t2->nil) {
                                y = y->right;
                        } else {
                                break;
                        }

                        if (y->color == RB_NODE_COLOR_BLACK) {
                                bh -= 1;
                        }
                }

                rb_tree_transplant(t2, y, x);
                x->left = t1->root;
                x->right = y;
                y->parent = x;
                t1->root->parent = x;

                rb_tree_insert_fixup(t2, x);

                rb_tree_copy(new_tree, t2);
        }

        free(t1);
        free(t2);

        return new_tree;
}

/**
 * @brief Split tree to t1, t2 based on key value x
 * 
 * @param tree split target tree
 * @param x split point
 * @param result1 t1 stored location
 * @param result2 t2 stored location
 * @return int If return value is 0 then success.
 * However, if return value is not 0 then failed.
 */
int rb_tree_split(struct rb_tree *tree, const key_t x, struct rb_tree **result1,
                  struct rb_tree **result2)
{
        struct rb_tree *t1 = NULL;
        struct rb_tree *t2 = NULL;

        struct rb_node *k = NULL;

        int ret = 0;

        t1 = rb_tree_alloc();
        if (!t1) {
                ret = -ENOMEM;
                goto exception;
        }
        t2 = rb_tree_alloc();
        if (!t2) {
                ret = -ENOMEM;
                goto exception;
        }

        k = tree->root;

        while (k != tree->nil) {
                struct rb_node *temp = k;
                if (x < k->key) {
                        if (k->right != tree->nil) {
                                __rb_tree_insert(t2, k->right);
                        }
                        rb_tree_insert(t2, k->key, k->data);
                        k = k->left;
                } else {
                        if (k->left != tree->nil) {
                                __rb_tree_insert(t1, k->left);
                        }
                        rb_tree_insert(t1, k->key, k->data);
                        k = k->right;
                }

                free(temp);
        }

        *result1 = t1;
        *result2 = t2;
        free(tree);
        return ret;
exception:
        if (t1) {
                rb_tree_dealloc(t1);
        }
        if (t2) {
                rb_tree_dealloc(t2);
        }
        *result1 = NULL;
        *result2 = NULL;
        return ret;
}

/**
 * @brief Does deallocation of the red-black tree subtree
 * 
 * @param tree red-black tree whole
 * @param node the root of the subtree
 */
static void __rb_tree_dealloc(struct rb_tree *tree, struct rb_node *node)
{
        if (!node || rb_node_is_leaf(tree, node)) {
                return;
        }

        __rb_tree_dealloc(tree, node->left);
        node->left = NULL;
        __rb_tree_dealloc(tree, node->right);
        node->right = NULL;

        rb_node_dealloc(node);
}

/**
 * @brief Does deallocation fo the red-black tree
 * 
 * @param tree red-black tree whole
 */
void rb_tree_dealloc(struct rb_tree *tree)
{
        __rb_tree_dealloc(tree, tree->root);
        tree->root = NULL;

        tree->nil = NULL;

        free(tree);
}

#ifdef RB_TREE_DEBUG
static void __rb_tree_dump(struct rb_tree *tree, struct rb_node *root,
                           size_t indent)
{
        const size_t INDENT_SIZE = 3;

        if (root == tree->nil) {
                return;
        }

        indent += INDENT_SIZE;

        __rb_tree_dump(tree, root->right, indent);

        printf("\n");
        for (size_t i = INDENT_SIZE; i < indent; i++) {
                printf(" ");
        }
        printf("%ld\n", root->key);

        __rb_tree_dump(tree, root->left, indent);
}

void rb_tree_dump(struct rb_tree *tree)
{
        __rb_tree_dump(tree, tree->root, 0);
}
#endif
