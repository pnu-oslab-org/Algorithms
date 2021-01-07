/**
 * @file btree.h
 * @author 오기준 (kijunking@pusan.ac.kr)
 * @brief B-Tree에 대한 선언적 내용이 들어가 있다.
 * @version 0.1
 * @date 2020-06-16
 * 
 * @copyright Copyright (c) 2020 오기준
 * 
 */
#ifndef _B_TREE_H
#define _B_TREE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef B_TREE_DEALLOC_ITEM /**< btree_dealloc_node의 warning 내용을 수정하기 전까지 유지할 것 */
#pragma message                                                                \
        "[WARN] I am not recommended to use dynamic allocation in `item->data`..."
#endif

#define B_TREE_MIN_DEGREE 2 /**< B-Tree의 최소 차수로 변경해서는 안된다. */
#define B_TREE_NOT_FOUND -1 /**< Node 값을 찾지 못한 경우에 사용한다. */

#define B_TREE_NR_CHILD(DEG) (2 * (DEG)) // 4(2-3-4), 3(2-3)
#define B_TREE_NR_KEYS(DEG) (B_TREE_NR_CHILD(DEG) - 1) // 3(2-3-4), 2(2-3)

#ifndef key_t
typedef unsigned int key_t;
#endif

#define pr_info(msg, ...)                                                      \
        fprintf(stderr, "[{%lfs} %s(%s):%d] " msg,                             \
                ((double)clock() / CLOCKS_PER_SEC), __FILE__, __func__,        \
                __LINE__, ##__VA_ARGS__)

/**
 * @brief B-Tree의 탐색에 사용되는 구조체로 이를 통해서 B-Tree 탐색 결과를 받을 수 있다.
 * 
 */
struct btree_search_result {
        int index;
        struct btree_node *node;
};

/**
 * @brief B-Tree의 노드가 가지는 항목에 해당한다.
 * 
 */
struct btree_item {
        key_t key;
        void *data;
};

/**
 * @brief B-Tree의 노드에 해당한다.
 * 
 */
struct btree_node {
        int n; /**< 노드가 현재 사용 중인 항목의 갯수를 가진다. */
        bool is_leaf; /**< 노드가 leaf 위치에 있는 지에 대한 정보를 가진다. */

        struct btree_item *items; /**< 항목들에 대한 데이터들을 가진다. */
        struct btree_node **child; /**< 자식에 대한 포인터들을 가진다. */
};

/**
 * @brief B-Tree 전체를 관리하는 구조체에 해당한다.
 * @note 반드시 생성될 때에 min_degree는 설정이 되어야 한다.
 * 
 */
struct btree {
        int min_degree; /**< 현재 B-Tree가 가지는 최소 차수를 가진다. */
        struct btree_node *root; /**< B-Tree의 루트 노드를 가리킨다. */
};

struct btree *btree_alloc(int min_degree);
struct btree_search_result btree_search(struct btree *tree, key_t key);
void btree_insert(struct btree *tree, key_t key, void *data);
void btree_traverse(struct btree *tree);
int btree_delete(struct btree *tree, key_t key);
void btree_free(struct btree *tree);

#endif