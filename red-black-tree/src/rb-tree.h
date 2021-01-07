/**
 * @file rb-tree.h
 * @author BlaCkinkGJ (ss5kijun@gmail.com)
 * @brief red black tree data structure's declaration part
 * @version 0.1
 * @date 2020-05-29
 * 
 * @copyright Copyright (c) 2020 BlaCkinkGJ
 * 
 * @ref Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009). Introduction to algorithms. MIT press.
 * 
 */
#ifndef RB_TREE_H_
#define RB_TREE_H_

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#ifdef key_t
#warning "already key_t is defined"
#else
typedef uint64_t key_t;
#endif

#define RB_INVALID_BLACK_HEIGHT (-1)
#define RB_MAX_KEY ((key_t)(LONG_MAX))
#define RB_NODE_NIL_KEY_VALUE (RB_MAX_KEY)

#ifndef pr_info
#define pr_info(msg, ...)                                                      \
        fprintf(stderr, "[{%lfs} %s(%s):%d] " msg,                             \
                ((double)clock() / CLOCKS_PER_SEC), __FILE__, __func__,        \
                __LINE__, ##__VA_ARGS__)
#endif

/**
 * @brief rb_node's color type
 * 
 */
enum rb_node_color {
        RB_NODE_COLOR_RED,
        RB_NODE_COLOR_BLACK,
        RB_NODE_COLOR_UNDEFINED,
};

/**
 * @brief Red black tree's node
 * 
 */
struct rb_node {
        enum rb_node_color color;

        key_t key;
        void *data; /**< must be allocated in HEAP location */

        struct rb_node *left, *right;
        struct rb_node *parent; /**< same as P in CLRS books */
};

struct rb_global_info {
        struct rb_node nil;
};

/**
 * @brief Red black tree structure
 * 
 */
struct rb_tree {
        struct rb_node *root;
        struct rb_node *nil; /**< same as Nil in CLRS books */
        size_t bh;
};

struct rb_tree *rb_tree_alloc(void);
struct rb_node *rb_tree_search(struct rb_tree *tree, key_t key);
size_t rb_tree_get_bh(struct rb_tree *tree, key_t key);
int rb_tree_insert(struct rb_tree *tree, const key_t key, void *data);
struct rb_node *rb_tree_minimum(struct rb_tree *tree, struct rb_node *root);
struct rb_node *rb_tree_maximum(struct rb_tree *tree, struct rb_node *root);
struct rb_node *rb_tree_successor(struct rb_tree *tree, struct rb_node *x);
struct rb_node *rb_tree_predecessor(struct rb_tree *tree, struct rb_node *y);
struct rb_tree *rb_tree_concat(struct rb_tree *t1, struct rb_tree *t2,
                               struct rb_node *x);
int rb_tree_split(struct rb_tree *tree, const key_t x, struct rb_tree **result1,
                  struct rb_tree **result2);
int rb_tree_delete(struct rb_tree *tree, key_t key);
void rb_tree_dealloc(struct rb_tree *tree);

#ifdef RB_TREE_DEBUG
void rb_tree_dump(struct rb_tree *tree);
#endif

/**
 * @brief copy the source tree's metadata to destination tree
 * 
 * @param dest destination(target) tree
 * @param src source(base) tree
 */
static inline void rb_tree_copy(struct rb_tree *dest, struct rb_tree *src)
{
        memcpy(dest, src, sizeof(struct rb_tree));
}

/**
 * @brief Node check if the node is equal to tree->nil
 * 
 * @param tree red-black tree whole
 * @param node nil test target node
 * @return true node is nil
 * @return false node is not nil
 */
static inline int rb_node_is_leaf(struct rb_tree *tree, struct rb_node *node)
{
        return tree->nil == node;
}

/**
 * @brief Generate new node
 * 
 * @param key node's key
 * @return struct rb_node* allocated node
 */
static inline struct rb_node *rb_node_alloc(const key_t key)
{
        struct rb_node *new_node = NULL;

        if (key >= RB_MAX_KEY) {
                pr_info("Invalid key value\n");
                return NULL;
        }

        new_node = (struct rb_node *)malloc(sizeof(struct rb_node));
        if (!new_node) {
                pr_info("Memory allocation failed\n");
                return NULL;
        }
        new_node->color = RB_NODE_COLOR_UNDEFINED;
        new_node->parent = new_node->left = new_node->right = NULL;
        new_node->data = NULL;

        new_node->key = key;

        return new_node;
}

/**
 * @brief Deallocation node
 * 
 * @param node deallocate target
 * @warning If reference counter is over 0 then node cannot be deallocated by system.
 */
static inline void rb_node_dealloc(struct rb_node *node)
{
        if (node->data) {
                free(node->data);
        }
        free(node);
}

/**
 * @brief Source's key and data to destination
 * 
 * @param dest destination node
 * @param source source node
 * @return struct rb_node* destination node
 * 
 * @warning This changes memory allocation state. So, you must carefully use this function.
 */
static inline struct rb_node *rb_node_move(struct rb_node *dest,
                                           struct rb_node *source)
{
        void *dest_data = dest->data;
        dest->key = source->key;
        dest->data = source->data;
        source->data = dest_data;

        rb_node_dealloc(source);

        return dest;
}
#endif