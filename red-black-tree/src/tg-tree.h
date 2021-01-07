/**
 * @file tg-tree.h
 * @author BlaCkinkGJ (ss5kijun@gmail.com)
 * @brief tango-tree data structure's declaration part
 * @version 0.1
 * @date 2020-06-08
 * 
 * @copyright Copyright (c) 2020 BlaCkinkGJ
 * 
 */
#ifndef TANGO_TREE_H_
#define TANGO_TREE_H_

#include <errno.h>
#include "rb-tree.h"

enum tg_bst_edge_prefer {
        TG_BST_LEFT,
        TG_BST_RIGHT,
        TG_BST_UNKNOWN,
};

struct tg_bst_node {
        key_t key;
        enum tg_bst_edge_prefer prefer;
        void *data;
        size_t depth;
        struct tg_bst_node *left;
        struct tg_bst_node *right;
        struct tg_bst_node *parent;
};

struct tg_bst_tree {
        struct tg_bst_node *root;
};

struct tg_node {
        size_t nr_nodes;
        struct rb_tree *aux;
};

struct tg_tree {
        struct tg_bst_tree *ref;
        struct tg_node *root;
};

struct tg_bst_tree *tg_bst_tree_alloc(void);
void tg_bst_tree_dealloc(struct tg_bst_tree *tree);
int tg_bst_tree_insert(struct tg_bst_tree *tree, key_t key, void *data);
struct tg_bst_node *tg_bst_tree_search(struct tg_bst_tree *tree, key_t key);
#ifdef TG_BST_TREE_DEBUG
void tg_bst_tree_dump(struct tg_bst_tree *tree);
#endif

struct tg_tree *tg_tree_alloc(key_t *keys, size_t size);
struct tg_node *tg_tree_search(struct tg_tree *tree, key_t key);
void tg_tree_cut(struct tg_tree *tree, key_t key);
struct tg_tree *tg_tree_join(struct tg_tree *tree, struct tg_node *aux1,
                             struct tg_node *aux2);
void tg_tree_dealloc(struct tg_tree *tree);

static inline struct tg_node *tg_node_alloc(void)
{
        struct tg_node *node = (struct tg_node *)malloc(sizeof(struct tg_node));
        if (!node) {
                pr_info("Node allocation failed...\n");
                goto exception;
        }

        node->aux = rb_tree_alloc();
        if (!node->aux) {
                pr_info("Preferred path consists failed...\n");
                goto exception;
        }

        return node;
exception:
        if (node->aux) {
                rb_tree_dealloc(node->aux);
        }
        if (node) {
                free(node);
        }
        return NULL;
}

static inline void tg_node_dealloc(struct tg_node *node)
{
        if (node->aux) {
                rb_tree_dealloc(node->aux);
                node->aux = NULL;
        }
        free(node);
}
#endif