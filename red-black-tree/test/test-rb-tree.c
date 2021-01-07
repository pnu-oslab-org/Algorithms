#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "rb-tree.h"
#include "unity.h"

#define INSERT_SIZE (1000)
#define STR_BUF_SIZE (256)
#define NR_TREE (2)

struct rb_tree *tree_arr[NR_TREE];
struct rb_tree *tree;
key_t *key_arr;
char **data_arr;

void setUp(void)
{
        for (int i = 0; i < NR_TREE; i++) {
                tree_arr[i] = rb_tree_alloc();
                TEST_ASSERT_NOT_NULL(tree_arr[i]);
        }

        tree = tree_arr[0];
        TEST_ASSERT_NOT_NULL(tree);

        key_arr = (key_t *)malloc(sizeof(key_t) * INSERT_SIZE);
        TEST_ASSERT_NOT_NULL(key_arr);
        data_arr = (char **)malloc(sizeof(char *) * INSERT_SIZE);
        TEST_ASSERT_NOT_NULL(data_arr);
}

void tearDown(void)
{
        for (int i = 0; i < NR_TREE; i++) {
                if (tree_arr[i] != NULL) {
                        rb_tree_dealloc(tree_arr[i]);
                }
        }

        free(key_arr);
        free(data_arr);
}

void test_rb_insert(void)
{
        for (int i = 0; i < INSERT_SIZE; i++) {
                key_arr[i] = rand() % INSERT_SIZE;
        }

        for (int i = 0; i < INSERT_SIZE; i++) {
                char *data = (char *)malloc(sizeof(char) * STR_BUF_SIZE);
                sprintf(data, "%d", i);
                data_arr[key_arr[i]] = data;
                TEST_ASSERT_EQUAL(0, rb_tree_insert(tree, key_arr[i], data));
        }
}

void test_rb_valid_search(void)
{
        test_rb_insert(); /**< Generate Test Cases */
        for (int i = 0; i < INSERT_SIZE; i++) {
                struct rb_node *node = rb_tree_search(tree, key_arr[i]);
                TEST_ASSERT_NOT_NULL(node);
                TEST_ASSERT_EQUAL(key_arr[i], node->key);
                TEST_ASSERT_EQUAL_STRING(data_arr[key_arr[i]], node->data);
        }
}

void test_rb_invalid_search(void)
{
        test_rb_insert(); /**< Generate Test Cases */
        for (key_t key = INSERT_SIZE; key < 2 * INSERT_SIZE; key++) {
                TEST_ASSERT_NULL(rb_tree_search(tree, key));
        }
        TEST_ASSERT_NOT_NULL(rb_tree_search(tree, key_arr[INSERT_SIZE - 1]));
}

void test_rb_minimum(void)
{
        key_t values[] = { 10, 35, 5, 22 };
        const int nr_values = (int)(sizeof(values) / sizeof(key_t));
        for (int i = 0; i < nr_values; i++) {
                TEST_ASSERT_EQUAL(0, rb_tree_insert(tree, values[i], NULL));
        }
        TEST_ASSERT_EQUAL(5, rb_tree_minimum(tree, tree->root)->key);
}

void test_rb_maximum(void)
{
        key_t values[] = { 10, 35, 5, 22 };
        const int nr_values = (int)(sizeof(values) / sizeof(key_t));
        for (int i = 0; i < nr_values; i++) {
                TEST_ASSERT_EQUAL(0, rb_tree_insert(tree, values[i], NULL));
        }
        TEST_ASSERT_EQUAL(35, rb_tree_maximum(tree, tree->root)->key);
}

void test_rb_successor_and_predecessor(void)
{
        struct rb_node *succ, *pred, *cur;
        int i = 0;
        key_t values[] = { 10, 35, 5, 22 };
        key_t expects[] = { RB_NODE_NIL_KEY_VALUE, 5, 10, 22, 35,
                            RB_NODE_NIL_KEY_VALUE };
        const int nr_expects = (int)(sizeof(expects) / sizeof(key_t));
        const int nr_values = (int)(sizeof(values) / sizeof(key_t));
        for (int i = 0; i < nr_values; i++) {
                TEST_ASSERT_EQUAL(0, rb_tree_insert(tree, values[i], NULL));
        }

        cur = rb_tree_minimum(tree, tree->root);
        for (i = 0; i < (nr_expects - 1) && cur != tree->nil; i++) {
                succ = rb_tree_successor(tree, cur);
                pred = rb_tree_predecessor(tree, cur);

                TEST_ASSERT_NOT_NULL(cur);
                TEST_ASSERT_NOT_NULL(succ);
                TEST_ASSERT_NOT_NULL(pred);

                cur = succ;

                TEST_ASSERT_EQUAL(expects[i], pred->key);
                TEST_ASSERT_EQUAL(expects[i + 2], succ->key);
        }
        TEST_ASSERT_EQUAL(tree->nil, cur);
        TEST_ASSERT_EQUAL(i, nr_expects - 2);
}

void test_rb_delete(void)
{
        struct rb_node *node;
        key_t values[] = { 10, 35, 5, 22 };
        const int nr_values = (int)(sizeof(values) / sizeof(key_t));
        for (int i = 0; i < nr_values; i++) {
                TEST_ASSERT_EQUAL(0, rb_tree_insert(tree, values[i], NULL));
        }
        for (int i = 0; i < nr_values; i++) {
                node = rb_tree_search(tree, values[i]);
                TEST_ASSERT_NOT_NULL(node);
                TEST_ASSERT_EQUAL(0, rb_tree_delete(tree, values[i]));
                node = rb_tree_search(tree, values[i]);
                TEST_ASSERT_NULL(node);
                TEST_ASSERT_EQUAL(-ENODATA, rb_tree_delete(tree, values[i]));
        }

        TEST_ASSERT_EQUAL_PTR(tree->nil, tree->root);
}

void test_rb_bh(void)
{
        key_t insert_seq[] = { 10, 20, 5,  7,  6,  19, 18,
                               17, 16, 15, 21, 22, 14, 13 };
        key_t delete_seq[] = { 10, 6,  5,  16, 7,  13, 15,
                               14, 21, 20, 22, 18, 19, 17 };
        key_t get_bh_seq[] = { 17, 10, 19, 6,  15, 18, 21,
                               5,  7,  14, 16, 20, 22, 13 };

        size_t insert_bh[] = { 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3 };
        size_t delete_bh[] = { 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1, 1, 0 };
        size_t get_bh[] = { 3, 2, 2, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0 };

        const int nr_inserts = (int)(sizeof(insert_seq) / sizeof(key_t));
        const int nr_deletes = (int)(sizeof(delete_seq) / sizeof(key_t));
        const int nr_get_bh = (int)(sizeof(get_bh_seq) / sizeof(key_t));

        for (int i = 0; i < nr_inserts; i++) {
                TEST_ASSERT_EQUAL(0, rb_tree_insert(tree, insert_seq[i], NULL));
                TEST_ASSERT_EQUAL(insert_bh[i], tree->bh);
        }

        for (int i = 0; i < nr_get_bh; i++) {
                TEST_ASSERT_EQUAL(get_bh[i],
                                  rb_tree_get_bh(tree, get_bh_seq[i]));
        }

        TEST_ASSERT_EQUAL(RB_INVALID_BLACK_HEIGHT, rb_tree_get_bh(tree, 55));

        for (int i = 0; i < nr_deletes; i++) {
                TEST_ASSERT_EQUAL(0, rb_tree_delete(tree, delete_seq[i]));
                TEST_ASSERT_EQUAL(delete_bh[i], tree->bh);
        }

        for (int i = 0; i < nr_inserts; i++) {
                TEST_ASSERT_EQUAL(0, rb_tree_insert(tree, insert_seq[i], NULL));
                TEST_ASSERT_EQUAL(insert_bh[i], tree->bh);
        }
}

void test_rb_concat(void)
{
        struct rb_tree *t1 = tree_arr[0];
        struct rb_tree *t2 = tree_arr[1];
        struct rb_node *x;

        key_t t1_data[] = { 1, 2, 3, 4, 5 };
        key_t t2_data[] = { 7, 8, 9, 10, 11 };

        const int nr_t1_data = (int)(sizeof(t1_data) / sizeof(key_t));
        const int nr_t2_data = (int)(sizeof(t2_data) / sizeof(key_t));

        for (int i = 0; i < nr_t1_data; i++) {
                TEST_ASSERT_EQUAL(0, rb_tree_insert(t1, t1_data[i], NULL));
        }

        for (int i = 0; i < nr_t2_data; i++) {
                TEST_ASSERT_EQUAL(0, rb_tree_insert(t2, t2_data[i], NULL));
        }

        x = rb_tree_minimum(t2, t2->root);
        tree = rb_tree_concat(t1, t2, x);
        if (tree == NULL) {
                rb_node_dealloc(x);
        } else {
                tree_arr[0] = NULL;
                tree_arr[1] = NULL;
        }
        TEST_ASSERT_NOT_NULL(tree);
        tree_arr[0] = tree;

        for (int i = 0; i < nr_t1_data; i++) {
                struct rb_node *find = rb_tree_search(tree, t1_data[i]);
                TEST_ASSERT_NOT_NULL(find);
                TEST_ASSERT_EQUAL(t1_data[i], find->key);
        }

        for (int i = 0; i < nr_t2_data; i++) {
                struct rb_node *find = rb_tree_search(tree, t2_data[i]);
                TEST_ASSERT_NOT_NULL(find);
                TEST_ASSERT_EQUAL(t2_data[i], find->key);
        }
}

void test_rb_split(void)
{
        struct rb_tree *tree = tree_arr[0];
        struct rb_tree *t1 = NULL;
        struct rb_tree *t2 = NULL;
        key_t tree_data[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
        const key_t SPLIT_POINT = 6;

        const int nr_tree_data = (int)(sizeof(tree_data) / sizeof(key_t));
        for (int i = 0; i < nr_tree_data; i++) {
                TEST_ASSERT_EQUAL(0, rb_tree_insert(tree, tree_data[i], NULL));
        }

        rb_tree_split(tree, SPLIT_POINT, &t1, &t2);
        for (int i = 0; i < nr_tree_data; i++) {
                size_t t1_addr = (size_t)rb_tree_search(t1, tree_data[i]);
                size_t t2_addr = (size_t)rb_tree_search(t2, tree_data[i]);
                TEST_ASSERT_NOT_EQUAL(0, (t1_addr | t2_addr));
                TEST_ASSERT_EQUAL(0, (t1_addr & t2_addr));
        }
        tree_arr[0] = NULL;
        rb_tree_dealloc(t1);
        rb_tree_dealloc(t2);
}

int main(void)
{
        UNITY_BEGIN();

        RUN_TEST(test_rb_insert);
        RUN_TEST(test_rb_valid_search);
        RUN_TEST(test_rb_invalid_search);
        RUN_TEST(test_rb_minimum);
        RUN_TEST(test_rb_maximum);
        RUN_TEST(test_rb_successor_and_predecessor);
        RUN_TEST(test_rb_delete);
        RUN_TEST(test_rb_bh);
        RUN_TEST(test_rb_concat);
        RUN_TEST(test_rb_split);

        return UNITY_END();
}