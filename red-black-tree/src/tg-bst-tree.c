/**
 * @file tg-bst-tree.c
 * @author BlaCkinkGJ (ss5kijun@gmail.com)
 * @brief Tango Tree's reference tree generator
 * @version 0.1
 * @date 2020-06-08
 * 
 * @copyright Copyright (c) 2020 BlaCkinkGJ
 * 
 */
#include "tg-tree.h"

/**
 * @brief calculate depth of the complete binary tree
 * 
 * @param tree complete binary tree
 * @param node start location
 * @return size_t depth of node
 * 
 * @warning only works when reference tree is complete binary tree
 */
size_t tg_bst_get_depth(struct tg_bst_tree *tree, struct tg_bst_node *node)
{
        size_t depth = 1;
        while (node->left) {
                depth++;
        }
        return depth;
}

struct tg_bst_tree *tg_bst_tree_alloc(void)
{
        struct tg_bst_tree *tree = NULL;

        tree = (struct tg_bst_tree *)malloc(sizeof(struct tg_bst_tree));
        if (!tree) {
                pr_info("Tree allocation failed...\n");
                return NULL;
        }
        tree->root = NULL;
        return tree;
}

static void __tg_bst_tree_dealloc(struct tg_bst_node *root)
{
        if (!root) {
                return;
        }

        __tg_bst_tree_dealloc(root->left);
        root->left = NULL;
        __tg_bst_tree_dealloc(root->right);
        root->right = NULL;

        if (root->data) {
                free(root->data);
        }
        free(root);
}

void tg_bst_tree_dealloc(struct tg_bst_tree *tree)
{
        if (tree->root) {
                __tg_bst_tree_dealloc(tree->root);
        }
        free(tree);
}

static int __tg_bst_tree_insert(struct tg_bst_tree *tree, struct tg_bst_node *z)
{
        struct tg_bst_node *y = NULL;
        struct tg_bst_node *x = tree->root;

        while (x != NULL) {
                y = x;
                if (z->key < x->key) {
                        x->prefer = TG_BST_LEFT;
                        x = x->left;
                } else {
                        x->prefer = TG_BST_RIGHT;
                        x = x->right;
                }
        }

        z->parent = y;
        if (y == NULL) {
                tree->root = z;
        } else if (z->key < y->key) {
                y->left = z;
        } else {
                y->right = z;
        }

        return 0;
}

int tg_bst_tree_insert(struct tg_bst_tree *tree, key_t key, void *data)
{
        struct tg_bst_node *node = NULL;
        int ret;

        node = (struct tg_bst_node *)malloc(sizeof(struct tg_bst_node));
        if (!node) {
                pr_info("tg_bst node allocation failed...\n");
                return -ENOMEM;
        }
        node->key = key;
        node->data = data;
        node->left = node->right = NULL;
        node->parent = NULL;
        node->prefer = TG_BST_UNKNOWN;

        ret = __tg_bst_tree_insert(tree, node);
        if (ret != 0) {
                free(node);
        }

        return ret;
}

static struct tg_bst_node *__tg_bst_tree_search(struct tg_bst_node *x,
                                                key_t key)
{
        while (x != NULL && key != x->key) {
                if (key < x->key) {
                        x = x->left;
                } else {
                        x = x->right;
                }
        }

        return x;
}

struct tg_bst_node *tg_bst_tree_search(struct tg_bst_tree *tree, key_t key)
{
        return __tg_bst_tree_search(tree->root, key);
}

#ifdef TG_BST_TREE_DEBUG
static void __tg_bst_tree_dump(struct tg_bst_tree *tree,
                               struct tg_bst_node *root, size_t indent)
{
        const size_t INDENT_SIZE = 3;

        if (root == NULL) {
                return;
        }

        indent += INDENT_SIZE;

        __tg_bst_tree_dump(tree, root->right, indent);

        printf("\n");
        for (size_t i = INDENT_SIZE; i < indent; i++) {
                printf(" ");
        }
        printf("%ld(%s)\n", root->key,
               (root->prefer == TG_BST_LEFT ? "LEFT" : "RIGHT"));

        __tg_bst_tree_dump(tree, root->left, indent);
}

void tg_bst_tree_dump(struct tg_bst_tree *tree)
{
        __tg_bst_tree_dump(tree, tree->root, 0);
}
#endif