/**
 * @file tg-tree.c
 * @author BlaCkinkGJ (ss5kijun@gmail.com)
 * @brief tango-tree implementation part
 * @version 0.1
 * @date 2020-06-08
 * 
 * @copyright Copyright (c) 2020 BlaCkinkGJ
 * 
 */

#include "tg-tree.h"

struct tg_tree *tg_tree_alloc(key_t *keys, size_t size)
{
        struct tg_tree *tree = (struct tg_tree *)malloc(sizeof(struct tg_tree));
        size_t i = 0;

        if (!tree) {
                pr_info("Tango tree allocation failed...\n");
                goto exception;
        }

        tree->ref = tg_bst_tree_alloc();
        if (!tree->ref) {
                pr_info("Reference tree allocation failed\n");
                goto exception;
        }

        for (i = 0; i < size; i++) {
                if (tg_bst_tree_insert(tree->ref, keys[i], NULL)) {
                        pr_info("Insert error occurred!!\n");
                        goto exception;
                }
        }
        tree->root = NULL;

        return tree;

exception:
        if (tree) {
                tg_tree_dealloc(tree);
        }
        return NULL;
}

static void __tg_tree_dealloc(struct tg_tree *tree, struct tg_node *node)
{
        if (!node) {
                return;
        }

        tg_node_dealloc(node);
}

void tg_tree_dealloc(struct tg_tree *tree)
{
        if (tree->ref) {
                tg_bst_tree_dealloc(tree->ref);
        }
        tree->ref = NULL;
        __tg_tree_dealloc(tree, tree->root);
        tree->root = NULL;
        free(tree);
}