#include "btree.h"
#include "unity.h"
#include <time.h>
#include <limits.h>

struct btree *tree;

// #define SHUFFLE
// #define RANDOM

#define MAX_SIZE 100000
#define REMAIN 5
#define TEST_LOOP 2
#define ARR_SIZE(T) ((int)(sizeof(T) / sizeof(key_t)))

key_t keys[MAX_SIZE] = { 0 };

static void seqential(key_t *keys, int n)
{
        for (int i = 0; i < n; i++) {
                keys[i] = i;
        }
}

#if defined(SHUFFLE)
static void shuffle(key_t *keys, int n)
{
        int choice = 0;
        int temp = 0;
        srand(time(NULL));
        for (int i = 0; i < n; i++) {
                choice = rand() % n;
                temp = keys[choice];
                keys[choice] = keys[i];
                keys[i] = temp;
        }
}
#elif defined(RANDOM)
#define RANDOM_SPACE (INT_MAX)

static void random(key_t *keys, int n)
{
        srand(time(NULL));
        for (int i = 0; i < n; i++) {
                keys[i] = rand() % RANDOM_SPACE;
        }
}
#endif
void setUp(void)
{
        seqential(keys, ARR_SIZE(keys));
#if defined(SHUFFLE)
        shuffle(keys, ARR_SIZE(keys));
#elif defined(RANDOM)
        random(keys, ARR_SIZE(keys));
#endif
}

void tearDown(void)
{
        btree_free(tree);
        tree = NULL;
}

static void test_tree(int min_degree)
{
        struct btree_search_result result;
        int ret;

        tree = btree_alloc(min_degree);
        TEST_ASSERT_NOT_NULL(tree);

        for (int i = 0; i < TEST_LOOP; i++) {
                for (int i = 0; i < ARR_SIZE(keys); i++) {
                        btree_insert(tree, keys[i], NULL);
                }
                for (int i = 0; i < ARR_SIZE(keys); i++) {
                        result = btree_search(tree, keys[i]);
                        TEST_ASSERT_NOT_NULL(result.node);
                }

                result = btree_search(tree, MAX_SIZE + 1);
                TEST_ASSERT_NULL(result.node);
                for (int i = 0; i < ARR_SIZE(keys) - REMAIN; i++) {
                        //int del = i;
                        //int del = ARR_SIZE(keys) - i - 1;
                        key_t del = keys[i];
                        result = btree_search(tree, del);
                        TEST_ASSERT_NOT_NULL(result.node);
                        ret = btree_delete(tree, del);
                        TEST_ASSERT_EQUAL(0, ret);
#ifndef RANDOM
                        result = btree_search(tree, del);
                        TEST_ASSERT_NULL(result.node);
#endif
                }
        }

        btree_traverse(tree);
}

void test_min_degree_50_tree(void)
{
        clock_t start = clock();
        test_tree(50);
        clock_t end = clock();
        printf("=======> %lfs\n", (double)(end - start) / CLOCKS_PER_SEC);
}

void test_min_degree_8_tree(void)
{
        clock_t start = clock();
        test_tree(8);
        clock_t end = clock();
        printf("=======> %lfs\n", (double)(end - start) / CLOCKS_PER_SEC);
}

void test_min_degree_5_tree(void)
{
        clock_t start = clock();
        test_tree(5);
        clock_t end = clock();
        printf("=======> %lfs\n", (double)(end - start) / CLOCKS_PER_SEC);
}

void test_min_degree_3_tree(void)
{
        clock_t start = clock();
        test_tree(3);
        clock_t end = clock();
        printf("=======> %lfs\n", (double)(end - start) / CLOCKS_PER_SEC);
}

void test_234_tree(void)
{
        clock_t start = clock();
        test_tree(2);
        clock_t end = clock();
        printf("=======> %lfs\n", (double)(end - start) / CLOCKS_PER_SEC);
}

int main(void)
{
        UNITY_BEGIN();
        RUN_TEST(test_234_tree);
        RUN_TEST(test_min_degree_3_tree);
        RUN_TEST(test_min_degree_5_tree);
        RUN_TEST(test_min_degree_8_tree);
        RUN_TEST(test_min_degree_50_tree);
        return UNITY_END();
}