#include "unity.h"
#include "tg-tree.h"

#define KEY_SIZE(keys) (sizeof(keys) / sizeof(key_t))

struct tg_tree *tree;
key_t keys[] = { 8, 12, 14, 13, 15, 10, 11, 9, 4, 2, 3, 1, 6, 7, 5 };

void setUp(void)
{
        tree = tg_tree_alloc(keys, KEY_SIZE(keys));
        TEST_ASSERT_NOT_NULL(tree);
}

void tearDown(void)
{
        tg_tree_dealloc(tree);
}

void test_tg_tree_search(void)
{
        tg_bst_tree_dump(tree->ref);
}

int main(void)
{
        UNITY_BEGIN();
        RUN_TEST(test_tg_tree_search);
        return UNITY_END();
}