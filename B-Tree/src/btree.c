/**
 * @file btree.c
 * @author 오기준 (kijunking@pusan.ac.kr)
 * @brief B-Tree에 대한 세부 구현이 적혀있다.
 * @version 0.1
 * @date 2020-06-16
 * @details B-Tree에 대한 구현은 Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009).
 * Introduction to algorithms. MIT press.(이하 CLRS)에 적혀있는 의사 코드(insert, search)와 
 * 설명(delete)을 기반으로 최대한 작성하였다.
 * 
 * @copyright Copyright (c) 2020 오기준
 * 
 */
#include <stdlib.h>
#include <errno.h>
#include "btree.h"

/**
 * @brief B-Tree에 들어갈 노드를 할당을 해주도록 한다.
 * 
 * @param T B-Tree 포인터에 해당한다.
 * @return struct btree_node* 노드에 대한 포인터를 반환한다.
 * @exception 동적 할당을 실패하는 경우에는 NULL이 반환된다.
 * 
 * @warning T->min_degree가 반드시 설정이 되어있어야 한다. 
 */
static struct btree_node *btree_alloc_node(struct btree *T)
{
        struct btree_node *node = NULL;
        const int nr_keys = B_TREE_NR_KEYS(T->min_degree);
        const int nr_child = B_TREE_NR_CHILD(T->min_degree);

        node = (struct btree_node *)malloc(sizeof(struct btree_node));
        if (!node) {
                pr_info("Node allocation failed...\n");
                goto exception;
        }
        node->n = 0;
        node->is_leaf = false;
        node->items =
                (struct btree_item *)calloc(nr_keys, sizeof(struct btree_item));
        if (!node->items) {
                pr_info("Node item allocation failed...\n");
                goto exception;
        }
        node->child = (struct btree_node **)calloc(nr_child,
                                                   sizeof(struct btree_node *));
        if (!node->child) {
                pr_info("Node child allocation failed...\n");
                goto exception;
        }
        return node;
exception:
        if (node->child) {
                free(node->child);
        }
        if (node->items) {
                free(node->items);
        }
        if (node) {
                free(node);
        }
        return NULL;
}

/**
 * @brief B-Tree에 대한 해제를 수행하도록 한다.
 * 
 * @param node 할당 해제를 진행하고자하는 B-Tree의 노드를 지칭한다. 
 * @warning 동적 할당을 하여 data를 관리하는 경우에는 dangling pointer가 발생할 가능성이 매우 높다.
 * 현재 있는 item에 대한 동적 할당 해제 시퀀스는 정확한 임시로 최대한 해제할 수 있도록 만든 것일 뿐이므로
 * 향후 관련해서 수정 및 보완이 필요할 것으로 보인다.
 */
static void btree_dealloc_node(struct btree_node *node)
{
        if (node != NULL) {
#ifdef B_TREE_DEALLOC_ITEM
                for (int i = 0; i < node->n; i++) {
                        if (node->items[i].data) {
                                free(node->items[i].data);
                        }
                }
#endif
                if (node->child) {
                        free(node->child);
                }
                if (node->items) {
                        free(node->items);
                }
                free(node);
        }
}

/**
 * @brief 새로운 B-Tree를 할당을 하도록 한다.
 * 
 * @param min_degree 노드가 가지는 최소 차수를 의미한다. 이 값이 2이면 2-3-4 트리에 해당한다.
 * @return struct btree* 정상 할당이 된 경우에는 B-Tree 주소가 반환된다.
 * @exception 동적 할당을 실패한 경우에는 NULL이 반환된다.
 * 
 * @warning 절대로 min_degree 값이 2 미만을 가지도록 만들어서는 안된다.
 */
struct btree *btree_alloc(int min_degree)
{
        struct btree *tree = NULL;
        struct btree_node *node = NULL;

        if (min_degree < B_TREE_MIN_DEGREE) {
                pr_info("Degree must over 2\n");
                return NULL;
        }

        tree = (struct btree *)malloc(sizeof(struct btree));
        if (!tree) {
                pr_info("Allocation tree failed\n");
                goto exception;
        }
        tree->min_degree = min_degree; /**< DO NOT CHANGE */

        node = btree_alloc_node(tree);
        if (!node) {
                pr_info("Allocation node failed\n");
                goto exception;
        }

        node->is_leaf = true;
        node->n = 0;

        tree->root = node;

        return tree;

exception:
        if (node) {
                btree_dealloc_node(node);
                tree->root = NULL;
        }

        if (tree) {
                free(tree);
        }

        return NULL;
}

/**
 * @brief B-Tree에 대한 탐색을 수행하도록 한다.
 * 
 * @param x B-Tree의 노드 탐색 시작 지점에 해당한다.
 * @param k 입력하고자하는 키에 해당한다.
 * @return struct btree_search_result B-Tree의 경우 하나의 노드에는 여러 개의
 * 키를 포함하기 때문에 노드의 주소 뿐만 아니라 인덱스도 필요로 한다. 따라서, 그 인덱스를
 * 반환해주도록 한다.
 * 
 * 만약 데이터를 찾지 못한 경우에는 result의 node가 NULL로 설정이 되고,
 * index도 미리 정의된 B_TREE_NOT_FOUND
 */
static struct btree_search_result __btree_search(struct btree_node *x, key_t k)
{
        int i = 0;
        struct btree_search_result result;
        while (i < x->n && k > x->items[i].key) {
                i = i + 1;
        }

        if (i < x->n && k == x->items[i].key) {
                result.index = i;
                result.node = x;
                return result;
        } else if (x->is_leaf) {
                result.index = B_TREE_NOT_FOUND;
                result.node = NULL;
                return result;
        } else {
                return __btree_search(x->child[i], k);
        }
}

/**
 * @brief B-Tree 탐색 함수의 래핑 함수에 해당한다.
 * 
 * @param tree B-Tree를 가리키는 포인터에 해당한다.
 * @param key 찾고자 하는 키에 해당한다.
 * @return struct btree_search_result 래핑된 함수에서 제공하는 결과값을 반환한다.
 */
struct btree_search_result btree_search(struct btree *tree, key_t key)
{
        return __btree_search(tree->root, key);
}

/**
 * @brief 임의의 노드 x에 대해서 2개의 노드로 분할하는 작업을 한다.
 * 
 * @param T B-Tree를 가리키는 포인터에 해당한다.
 * @param x 분할이 발생하는 노드에 해당한다.
 * @param i 분할의 위치에 해당한다.
 */
static void btree_split_child(struct btree *T, struct btree_node *x, int i)
{
        const int t = T->min_degree;

        struct btree_node *z = btree_alloc_node(T);
        struct btree_node *y = x->child[i - 1];

        z->is_leaf = y->is_leaf;
        z->n = t - 1;

        for (int j = 0; j < t - 1; j++) {
                z->items[j] = y->items[j + t];
        }

        if (!y->is_leaf) {
                for (int j = 0; j < t; j++) {
                        z->child[j] = y->child[j + t];
                }
        }

        y->n = t - 1;

        for (int j = x->n; j >= i; j--) {
                x->child[j + 1] = x->child[j];
        }
        x->child[i] = z;

        for (int j = x->n; j >= i; j--) {
                x->items[j] = x->items[j - 1];
        }
        x->items[i - 1] = y->items[t - 1];
        x->n = x->n + 1;
}

/**
 * @brief 노드가 꽉 차지 않은 경우에 데이터의 삽입을 수행한다.
 * 
 * @param T B-Tree를 가리키는 포인터에 해당한다.
 * @param x 꽉 차지 않은 노드에 해당하는 포인터이다.
 * @param k 삽입 하고자 하는 항목에 해당한다.
 */
static void btree_insert_non_full(struct btree *T, struct btree_node *x,
                                  struct btree_item *k)
{
        int i = x->n;

        if (x->is_leaf) {
                while (i >= 1 && k->key < x->items[i - 1].key) {
                        x->items[i] = x->items[i - 1];
                        i = i - 1;
                }
                x->items[i] = *k;
                x->n = x->n + 1;
        } else {
                while (i >= 1 && k->key < x->items[i - 1].key) {
                        i = i - 1;
                }
                if (x->child[i]->n == B_TREE_NR_KEYS(T->min_degree)) {
                        btree_split_child(T, x, i + 1);
                        if (k->key > x->items[i].key) {
                                i = i + 1;
                        }
                }
                btree_insert_non_full(T, x->child[i], k);
        }
}

/**
 * @brief B-Tree에 대한 데이터의 삽입을 수행하도록 한다.
 * 
 * @param T B-Tree를 가리키는 포인터에 해당한다.
 * @param k 입력하고자하는 데이터에 해당한다.
 */
static void __btree_insert(struct btree *T, struct btree_item *k)
{
        struct btree_node *r = T->root;
        if (r->n == B_TREE_NR_KEYS(T->min_degree)) {
                struct btree_node *s = btree_alloc_node(T);
                T->root = s;
                s->is_leaf = false;
                s->n = 0;
                s->child[0] = r;

                btree_split_child(T, s, 1);
                btree_insert_non_full(T, s, k);
        } else {
                btree_insert_non_full(T, r, k);
        }
}

/**
 * @brief B-Tree의 삽입을 수행하는 함수에 대한 래핑 함수이다.
 * 
 * @param tree B-Tree를 가리키는 포인터에 해당한다.
 * @param key 입력하고자 하는 데이터의 키에 해당한다.
 * @param data 키와 함께 입력되고자 하는 데이터에 해당한다.
 * 
 * @note 아직까지 동적 할당 data를 제대로 지원하지 못하므로,
 * data는 정수와 실수같은 스칼라 타입을 사용해야 한다.
 * 
 * 이를 테면, `*((int *)data) = 1234;` 후에 `btree_insert(..,data)`
 * 와 같이 사용하면 된다.
 */
void btree_insert(struct btree *tree, key_t key, void *data)
{
        struct btree_item item = { .key = key, .data = data };
        __btree_insert(tree, &item);
}

/**
 * @brief 디버깅용으로 사용하는 함수로 이를 사용하면 B-Tree 전체를
 * 콘솔에 그릴 수 있다.
 * 
 * @param node 그리기 시작하는 노드의 위치에 해당한다.
 * @param indent 콘솔의 가장 좌측에서 얼마나 떨어지는 가에 대한 명세이다.
 */
void __btree_traverse(struct btree_node *node, int indent)
{
        if (node == NULL) {
                return;
        }

        for (int i = 0; i < indent; i++) {
                printf("\t");
        }

        for (int i = 0; i < node->n; i++) {
                printf("%d ", node->items[i].key);
        }
        printf("(%d)\n", node->n);

        for (int i = 0; i <= node->n; i++) {
                __btree_traverse(node->child[i], indent + 1);
        }
}

/**
 * @brief B-Tree 전체를 그리는 함수의 래핑 함수이다.
 * 
 * @param tree B-Tree의 포인터에 해당한다.
 */
void btree_traverse(struct btree *tree)
{
        __btree_traverse(tree->root, 0);
}

/**
 * @brief 임의의 노드에 노드 자신 포함해서 자식까지 전체 해제를 수행하도록 한다.
 * 
 * @param node 삭제 시작점에 해당한다.
 */
static void __btree_clear(struct btree_node *node)
{
        if (node) {
                if (!node->is_leaf) {
                        for (int i = 0; i < (node->n + 1); i++) {
                                __btree_clear(node->child[i]);
                        }
                }
                btree_dealloc_node(node);
        }
}

/**
 * @brief B-Tree에 대한 노드의 삭제를 수행하도록 한다.
 * 
 * @param tree B-Tree를 가리키는 포인터에 해당한다.
 */
static void btree_clear(struct btree *tree)
{
        __btree_clear(tree->root);
        tree->root = NULL;
}

/**
 * @brief 임의의 노드에서의 전위 값을 찾는 역할을 한다.
 * 
 * @param node 전위 값을 찾고자 하는 임의의 노드를 의미한다.
 * @return struct btree_item 전위에 해당하는 값을 반환한다.
 */
static struct btree_item btree_get_predecessor(struct btree_node *node)
{
        while (!node->is_leaf) {
                node = node->child[node->n];
        }
        return node->items[node->n - 1];
}

/**
 * @brief 임의의 노드에서의 후위 값을 찾는 역할을 한다.
 * 
 * @param node 후위 값을 찾고자 하는 임의의 노드를 의미한다.
 * @return struct btree_item 후위에 해당하는 값을 반환한다.
 */
static struct btree_item btree_get_successor(struct btree_node *node)
{
        while (!node->is_leaf) {
                node = node->child[0];
        }
        return node->items[0];
}

/**
 * @brief 임의의 노드에 대해서 병합을 실시하도록 한다.
 * @details i 위치의 왼쪽 자식에 부모의 i 내용과 오른쪽 자식의 내용을 병합을 하도록 한다.
 * 
 * @param T B-Tree에 대한 포인터를 가진다.
 * @param p 부모 노드의 위치에 해당한다.
 * @param i 부모 노드의 병합 위치에 해당한다.
 */
static void btree_merge_child(struct btree *T, struct btree_node *p, int i)
{
        const int t = T->min_degree;
        struct btree_node *child[] = {
                p->child[i],
                p->child[i + 1],
        };

        child[0]->n = B_TREE_NR_KEYS(T->min_degree);
        child[0]->items[t - 1] = p->items[i];

        for (int j = 0; j < t - 1; j++) {
                child[0]->items[j + t] = child[1]->items[j];
        }

        if (!child[0]->is_leaf) {
                for (int j = 0; j < t; j++) {
                        child[0]->child[j + t] = child[1]->child[j];
                }
        }

        p->n -= 1;

        for (int j = i; j < p->n; j++) {
                p->items[j] = p->items[j + 1];
                p->child[j + 1] = p->child[j + 2];
        }

        btree_dealloc_node(child[1]);
        if (p->n == 0) {
                btree_dealloc_node(p);
                if (p == T->root) {
                        T->root = child[0];
                }
        }
}

/**
 * @brief key에 해당하는 것에 대해서 재귀적으로 제거를 진행하도록 한다.
 * @details 각 case에 대한 내용은 CLRS를 참고하도록 한다.
 * 단, case 3의 경우에는 그 구분이 불명확하여 따로 a, b에 대해서 정의하지 않았다.
 * 
 * 또한, goto를 사용하여 각 case의 종료를 처리한 이유는 좀 더 편한 debugging을 위해 둔 것으로
 * 만약에 case 별 동작에 대한 상세를 알고 싶은 경우에는 btree_traverse()를 end 라벨 뒤에 붙여
 * 넣어보면 알 수 있다.
 * 
 * @param T B-Tree의 포인터에 해당한다.
 * @param x 현재 삭제를 수행하는 노드에 해당한다.
 * @param key 제거하고자 하는 위치의 키에 해당한다.
 * @return int 성공 시에 0을 반환한다.
 */
static int __btree_delete(struct btree *T, struct btree_node *x, key_t key)
{
        const int t = T->min_degree;
        int i = 0;

        while (i < x->n && key > x->items[i].key) {
                i = i + 1;
        }

        if (i < x->n && key == x->items[i].key) {
                if (x->is_leaf) { /**< case 1 */
                        x->n -= 1;
                        for (; i < x->n; i++) {
                                x->items[i] = x->items[i + 1];
                        }
                        goto end;
                } else { /**< case 2 */
                        struct btree_node *prev = x->child[i];
                        struct btree_node *next = x->child[i + 1];
                        if (prev->n >= t) { /**< case 2a */
                                struct btree_item prev_item = { 0 };

                                prev_item = btree_get_predecessor(prev);
                                __btree_delete(T, prev, prev_item.key);
                                x->items[i] = prev_item;

                                goto end;
                        } else if (next->n >= t) { /**< case 2b */
                                struct btree_item next_item = { 0 };

                                next_item = btree_get_successor(next);
                                __btree_delete(T, next, next_item.key);
                                x->items[i] = next_item;
                                goto end;
                        } else { /**< case 2c */
                                btree_merge_child(T, x, i);
                                __btree_delete(T, prev, key);
                        }
                }
        } else { /**< case 3 */
                struct btree_node *child = x->child[i];
                if (child->n == t - 1) {
                        struct btree_node *left = NULL;
                        struct btree_node *right = NULL;
                        int j = 0;

                        if (i > 0) { /**< get left child */
                                left = x->child[i - 1];
                        }

                        if (i < x->n) { /**< get right child */
                                right = x->child[i + 1];
                        }

                        if (left && left->n >= t) {
                                for (j = child->n; j > 0; --j) {
                                        child->items[j] = child->items[j - 1];
                                }
                                child->items[0] = x->items[i - 1];

                                if (!left->is_leaf) {
                                        for (j = child->n + 1; j > 0; j--) {
                                                child->child[j] =
                                                        child->child[j - 1];
                                        }
                                        child->child[0] = left->child[left->n];
                                }

                                child->n += 1;
                                x->items[i - 1] = left->items[left->n - 1];
                                left->n -= 1;

                        } else if (right && right->n >= t) {
                                child->items[child->n] = x->items[i];
                                child->n += 1;

                                x->items[i] = right->items[0];
                                right->n -= 1;

                                for (j = 0; j < right->n; j++) {
                                        right->items[j] = right->items[j + 1];
                                }

                                if (!right->is_leaf) {
                                        child->child[child->n] =
                                                right->child[0];
                                        for (j = 0; j <= right->n; j++) {
                                                right->child[j] =
                                                        right->child[j + 1];
                                        }
                                }
                        } else if (left) {
                                btree_merge_child(T, x, i - 1);
                                child = left;
                        } else if (right) {
                                btree_merge_child(T, x, i);
                        } // end of left, right adjust
                } // child[i] has "t-1" keys
                __btree_delete(T, child, key);
        } // end of find key location

end:
        return 0;
}

/**
 * @brief 삭제를 수행하는 함수의 래핑 함수에 해당한다.
 * 
 * @param tree 트리를 가리키는 포인터에 해당한다.
 * @param key 삭제를 하고자 하는 키에 해당한다.
 * @return int 삭제를 성공한 경우에는 0을 반환한다.
 * 
 * @todo 현재는 btree_search를 통해서 데이터의 존재 여부를 판단하고 삭제를 진행한다.
 * 하지만 이러한 방식의 경우에는 O(t*log_{t}(n))(t는 키의 갯수)의 시간을 필요로 하기 때문에
 * 오버헤드가 어느 정도 있는 편이다.
 * 
 * 추후에는 bloom filter나 다른 것을 활용해서 성능을 올리는 과정이 필요할 것으로 사료된다.
 */
int btree_delete(struct btree *tree, key_t key)
{
        struct btree_node *node = NULL;
        struct btree_node *root = tree->root;

        node = (btree_search(tree, key)).node;
        if (!node) {
                pr_info("Cannot find specific node\n");
                return -EINVAL;
        }

        if (root->n == 0 && root->is_leaf) {
                btree_clear(tree);
                return 0;
        }

        return __btree_delete(tree, root, key);
}

/**
 * @brief 동적 할당된 B-Tree를 해제한다.
 * 
 * @param tree 동적 할당된 B-Tree 포인터에 해당한다.
 */
void btree_free(struct btree *tree)
{
        if (tree) {
                struct btree_node *root = tree->root;
                while (root->n > 0) {
                        key_t key = root->items[0].key;
                        btree_delete(tree, key);
                        root = tree->root;
                }
                btree_dealloc_node(tree->root);
                free(tree);
        }
}
