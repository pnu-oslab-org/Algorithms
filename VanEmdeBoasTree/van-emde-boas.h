/**
 * @file van-emde-boas.h
 * @author 오기준 (kijunking@pusan.ac.kr)
 * @brief 반 엠데 보아스 트리에 관련된 구조체와 각종 함수에 대한 원형을 가지고 있는 헤더 파일이다.
 * @version 0.1
 * @date 2020-05-14
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef _VAN_EMDE_BOAS_H
#define _VAN_EMDE_BOAS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define pr_info(msg, ...)                                                      \
        fprintf(stderr, "[{%lfs} %s(%s):%d] " msg,                             \
                ((double)clock() / CLOCKS_PER_SEC), __FILE__, __func__,        \
                __LINE__, ##__VA_ARGS__)

#define NIL (-1)

#define vEB_root_up(u) ((int)pow(2, ceil(log2(u) / 2.0)))
#define vEB_root_down(u) ((int)pow(2, floor(log2(u) / 2.0)))

#define vEB_high(u, x) ((int)floor(x / vEB_root_down(u)))
#define vEB_low(u, y) (x % vEB_root_down(u))
#define vEB_index(u, x, y) (x * vEB_root_down(u) + y)

/**
 * @brief Generic한 swap 매크로에 대한 정의이다. (float은 현재 미지원)
 * 
 */
#define vEB_swap(a, b)                                                         \
        do {                                                                   \
                unsigned char                                                  \
                        temp[sizeof(a) == sizeof(b) ? (signed)sizeof(a) : -1]; \
                memcpy(temp, &b, sizeof(a));                                   \
                memcpy(&b, &a, sizeof(a));                                     \
                memcpy(&a, temp, sizeof(a));                                   \
        } while (0)

/**
 * @brief vEB(u)에 해당하는 구조체이다.
 * 
 */
struct vEB {
        int u; /**< 현재 집합의 크기 */
        int min; /**< 1이 표기된 최소 지점 (-1은 Nil과 동일) */
        int max; /**< 1이 표기된 최대 지점 (-1은 Nil과 동일) */
        struct vEB *summary; /**< summary 정보가 들어간다. */
        struct vEB *cluster[0]; /**< 다음 cluster 값이 들어가게 된다. */
};

struct vEB *vEB_init(const int u);
void vEB_tree_insert(struct vEB *V, int x);
bool vEB_tree_member(struct vEB *V, const int x);
int vEB_tree_successor(struct vEB *V, const int x);
int vEB_tree_predecessor(struct vEB *V, const int x);
void vEB_tree_delete(struct vEB *V, int x);
void vEB_free(struct vEB *v);

/**
 * @brief 집합의 크기를 2의 거듭제곱 꼴로 나타낼 수 있도록 만든다.
 * 
 * @param u 집합의 크기를 나타낸다.
 * @return int vEB를 지원할 수 있도록 2^n 꼴로 나타내어지는 값이다.
 */
static inline int vEB_get_valid_size(const int u)
{
        const bool is_power_of_two = u && (!(u & (u - 1)));
        if (u <= 0) {
                return NIL;
        }

        if (is_power_of_two) {
                return u;
        }

        return ((int)pow(2, ceil(log2(u))));
}

/**
 * @brief vEB 노드가 비어있는 경우에 삽입이 발생한 경우에 min = max = x를 수행한다.
 * 
 * @param V 특정 vEB 노드를 나타낸다.
 * @param x 삽입하고자하는 원소를 지칭한다.
 */
static inline void vEB_empty_tree_insert(struct vEB *V, const int x)
{
        V->min = V->max = x;
}

/**
 * @brief vEB 노드의 최솟값을 반환받도록 한다.
 * 
 * @param V 특정 vEB 노드를 나타낸다.
 * @return int 노드의 최솟값을 나타낸다.
 */
static inline int vEB_tree_minimum(struct vEB *V)
{
        return V->min;
}

/**
 * @brief vEB 노드의 최댓값을 반환하도록 한다.
 * 
 * @param V 특정 vEB 노드를 나타낸다.
 * @return int 노드의 최댓값을 나타낸다.
 */
static inline int vEB_tree_maximum(struct vEB *V)
{
        return V->max;
}

#ifdef __cplusplus
}
#endif

#endif