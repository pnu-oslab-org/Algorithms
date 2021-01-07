/**
 * @file van-emde-boas.c
 * @author 오기준 (kijunking@pusan.ac.kr)
 * @brief 반 엠데 보아스 트리에 관련된 함수에 대한 정의를 가지고 있다.
 * @version 0.1
 * @date 2020-05-14
 * @note 본 내용의 구현은 Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009).
 * Introduction to algorithms. MIT press.에서 제시한 알고리즘의 pseudo code에 기반하여 작성하였다.
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "van-emde-boas.h"

/**
 * @brief 반 엠데 보아스 트리의 초기화를 진행하는 핵심 구문이다.
 * 
 * @param u 반 엠데 보아스 트리의 집합 크기를 이야기한다.
 * @return struct vEB* 초기화된 반 엠데 보아스 트리를 반환하도록 한다.
 */
static struct vEB *__vEB_init(int u)
{
        struct vEB *v = NULL;
        const int nr_cluster = vEB_root_up(u);
        const int cluster_size = sizeof(struct vEB *) * nr_cluster;
        int i;

        v = (struct vEB *)malloc(sizeof(struct vEB) + cluster_size);
        if (v == NULL) {
                pr_info("Allocation failed\n");
                goto exception;
        }

        v->u = u;
        v->min = v->max = NIL;

        if (v->u > 2) {
                v->summary = __vEB_init(vEB_root_up(u));
                for (i = 0; i < nr_cluster; i++) {
                        v->cluster[i] = __vEB_init(vEB_root_down(u));
                }
        } else {
                v->summary = NULL;
                for (i = 0; i < nr_cluster; i++) {
                        v->cluster[i] = NULL;
                }
        }

exception:
        return v;
}

/**
 * @brief 집합 크기를 받아 유효한 집합 크기가 아닌 경우에 유효한 집합 크기로 변경을 해주도록 한다.
 * 
 * @param u 집합의 크기를 나타낸다.
 * @return struct vEB* 초기화된 반 엠데 보아스 트리를 반환한다.
 */
struct vEB *vEB_init(const int u)
{
        int pow_of_2_u = vEB_get_valid_size(u);
        if (pow_of_2_u == NIL) {
                pr_info("Invalid set size : %d\n", u);
        }
        return __vEB_init(pow_of_2_u);
}

/**
 * @brief vEB 트리에 특정 값 x를 삽입을 하는 과정이다.
 * 
 * @param V 반 엠데 보아스 트리의 포인터를 가진다.
 * @param x 삽입을 하고자 하는 값에 해당한다.
 */
void vEB_tree_insert(struct vEB *V, int x)
{
        int u = V->u;
        if (V->min == NIL) {
                vEB_empty_tree_insert(V, x);
        } else {
                if (x < V->min) {
                        vEB_swap(x, V->min);
                } // end of x < V->min
                if (V->u > 2) {
                        if (vEB_tree_minimum(V->cluster[vEB_high(u, x)]) ==
                            NIL) {
                                vEB_tree_insert(V->summary, vEB_high(u, x));
                                vEB_empty_tree_insert(
                                        V->cluster[vEB_high(u, x)],
                                        vEB_low(u, x));
                        } else {
                                vEB_tree_insert(V->cluster[vEB_high(u, x)],
                                                vEB_low(u, x));
                        } // end of vEB_tree_minimum
                } // end of V->u > 2
                if (x > V->max) {
                        V->max = x;
                } // end of x > V->max
        } // end of x > V->max
} // end of V->min == NIL

/**
 * @brief x 위치에 값이 존재하는 지를 확인하는 함수이다.
 * 
 * @param V 반 엠데 보아스 트리의 포인터를 가진다.
 * @param x 값이 존재하는 지 확인하고 싶은 위치입니다.
 * @return true 데이터를 찾은 경우 반환되는 값이다.
 * @return false 데이터를 찾지 못한 경우 반환되는 값이다
 */
bool vEB_tree_member(struct vEB *V, const int x)
{
        if (x == V->min || x == V->max) {
                return true;
        } else if (V->u == 2) {
                return false;
        } // end of x == V->min || x == V->max

        return vEB_tree_member(V->cluster[vEB_high(V->u, x)], vEB_low(V->u, x));
}

/**
 * @brief 직후 원소를 찾는 함수이다.
 * 
 * @param V 반 엠데 보아스 트리의 포인터이다.
 * @param x 직후 원소를 찾기 위한 기준 위치이다.
 * @return int 직후 원소의 위치이다.
 */
int vEB_tree_successor(struct vEB *V, const int x)
{
        if (V->u == 2) {
                return ((x == 0 && V->max == 1) ? 1 : NIL);
        } else if (V->min != NIL && x < V->min) {
                return V->min;
        } else {
                const int max_low =
                        vEB_tree_maximum(V->cluster[vEB_high(V->u, x)]);
                if (max_low != NIL && vEB_low(V->u, x) < max_low) {
                        const int offset = vEB_tree_successor(
                                V->cluster[vEB_high(V->u, x)],
                                vEB_low(V->u, x));
                        return vEB_index(V->u, vEB_high(V->u, x), offset);
                } else {
                        const int succ_cluster = vEB_tree_successor(
                                V->summary, vEB_high(V->u, x));
                        if (succ_cluster == NIL) {
                                return NIL;
                        } else {
                                const int offset = vEB_tree_minimum(
                                        V->cluster[succ_cluster]);
                                return vEB_index(V->u, succ_cluster, offset);
                        } // end of succ_cluster == NIL
                } // end of max_low != NIL && ~~
        } // end of V->u == 2
}

/**
 * @brief 직전 원소를 찾는 함수이다.
 * 
 * @param V 반 엠데 보아스 트리의 포인터이다.
 * @param x 직전 원소를 찾기 위한 기준 위치이다.
 * @return int 직전 원소의 위치이다.
 */
int vEB_tree_predecessor(struct vEB *V, const int x)
{
        if (V->u == 2) {
                return ((x == 1 && V->min == 0) ? 0 : NIL);
        } else if (V->max != NIL && x > V->max) {
                return V->max;
        } else {
                const int min_low =
                        vEB_tree_minimum(V->cluster[vEB_high(V->u, x)]);
                if (min_low != NIL && vEB_low(V->u, x) > min_low) {
                        const int offset = vEB_tree_predecessor(
                                V->cluster[vEB_high(V->u, x)],
                                vEB_low(V->u, x));
                        return vEB_index(V->u, vEB_high(V->u, x), offset);
                } else {
                        const int pred_cluster = vEB_tree_predecessor(
                                V->summary, vEB_high(V->u, x));
                        if (pred_cluster == NIL) {
                                if (V->min != NIL && x > V->min) {
                                        return V->min;
                                } else {
                                        return NIL;
                                }
                        } else {
                                const int offset = vEB_tree_maximum(
                                        V->cluster[pred_cluster]);
                                return vEB_index(V->u, pred_cluster, offset);
                        } // end of pred_cluster == NIL
                } // end of min_low != NIL && ~~
        } // end of V->u == 2
}

void vEB_tree_delete(struct vEB *V, int x)
{
        if (V->min == V->max) {
                V->max = V->min = NIL;
        } else if (V->u == 2) {
                V->max = V->min = (x == 0 ? 1 : 0);
        } else {
                if (x == V->min) {
                        int first_cluster = vEB_tree_minimum(V->summary);
                        x = vEB_index(
                                V->u, first_cluster,
                                vEB_tree_minimum(V->cluster[first_cluster]));
                        V->min = x;
                } // end of x == V->min
                vEB_tree_delete(V->cluster[vEB_high(V->u, x)],
                                vEB_low(V->u, x));

                if (vEB_tree_minimum(V->cluster[vEB_high(V->u, x)]) == NIL) {
                        vEB_tree_delete(V->summary, vEB_high(V->u, x));
                        if (x == V->max) {
                                const int summary_max =
                                        vEB_tree_maximum(V->summary);
                                if (summary_max == NIL) {
                                        V->max = V->min;
                                } else {
                                        V->max = vEB_index(
                                                V->u, summary_max,
                                                vEB_tree_maximum(
                                                        V->cluster[summary_max]));
                                } // end fo summary_max == NIL
                        } // end of x == V->max
                } else if (x == V->max) {
                        V->max = vEB_index(
                                V->u, vEB_high(V->u, x),
                                vEB_tree_maximum(
                                        V->cluster[vEB_high(V->u, x)]));
                } // end of vEB_tree_minimum
        } // end of V->min == V->max
}

/**
 * @brief 할당된 반 엠데 보아스 트리를 해제하도록 한다.
 * 
 * @param V 이전에 할당된 반 엠데 보아스 트리의 포인터를 의미한다.
 */
void vEB_free(struct vEB *V)
{
        const int nr_cluster = vEB_root_up(V->u);
        int i;

        if (V->summary == NULL) {
                goto dealloc;
        }
        vEB_free(V->summary);
        V->summary = NULL;

        for (i = 0; i < nr_cluster; i++) {
                if (V->cluster[i] != NULL) {
                        vEB_free(V->cluster[i]);
                        V->cluster[i] = NULL;
                }
        }

dealloc:
        free(V);
}