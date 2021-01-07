/**
 * @file dynamic-array.h
 * @author 오기준 (kijunking@pusan.ac.kr)
 * @brief dynamic array에 대한 선언적 내용이 들어간다.
 * @version 0.1
 * @date 2020-05-18
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef _DYNAMIC_ARRAY_H
#define _DYNAMIC_ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
#define SIZE_T_FORMAT "%I64d"
#define KEY_FORMAT SIZE_T_FORMAT
typedef size_t key_t;
#endif

#ifdef linux
#define SIZE_T_FORMAT "%ld"
#define KEY_FORMAT "%d"
#include <sys/types.h> // for key_t
#endif

typedef unsigned long bitmap_t;

#define BITS (8)
#define BITS_PER_BITMAP (sizeof(bitmap_t) * BITS)
#define BITMAP_SIZE(nr_lines) (((nr_lines) / BITS_PER_BITMAP) + 1)

#define INDEX_EMPTY ((size_t)(~(size_t)0UL))

#define FREE_DISABLE 0
#define FREE_ENABLE 1

#define TEST
// #define GENERIC_COUNTER
#define BIT_COUNTER

#ifdef TEST
#if defined(GENERIC_COUNTER)
struct counter {
        int alloc;
        int dealloc;
        int insert;
        int insert_iter;
        int search;
        int search_iter;
        int compare;
};
#elif defined(BIT_COUNTER)
struct counter {
        size_t size;
        int *bit_set_counter;
        int *bit_unset_counter;
};
#endif

struct counter counter;

#if defined(GENERIC_COUNTER)
#define PRINT_HEADER()                                                         \
        (printf("alloc\tdealloc\tinsert\titer\tsearch\titer\tcmp\n"))

#define PRINT_COUNTER()                                                        \
        do {                                                                   \
                printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\n", counter.alloc,          \
                       counter.dealloc, counter.search, counter.search_iter,   \
                       counter.insert, counter.insert_iter, counter.compare);  \
        } while (0);
#elif defined(BIT_COUNTER)
#define PRINT_HEADER() (printf("bits\n"))

static inline void PRINT_COUNTER()
{
        printf("set\t");
        for (int i = 0; i < counter.size; i++) {
                printf("%d ", counter.bit_set_counter[i]);
        }
        printf("\n");
        printf("unset\t");
        for (int i = 0; i < counter.size; i++) {
                printf("%d ", counter.bit_unset_counter[i]);
        }
        printf("\n");
}
#endif
#endif

/**
 * @brief debug용 출력 함수
 * 
 */
#define pr_info(msg, ...)                                                      \
        fprintf(stderr, "[{%lfs} %s(%s):%d] " msg,                             \
                ((double)clock() / CLOCKS_PER_SEC), __FILE__, __func__,        \
                __LINE__, ##__VA_ARGS__)

/**
 * @brief bitmap에 대한 bit 연산을 수행한다.
 * 
 */
#define dynamic_array_set_bit(bitmap, index)                                   \
        ((bitmap)[(bitmap_t)((index) / BITS_PER_BITMAP)] |=                    \
         (1 << ((index) % BITS_PER_BITMAP)))
#define dynamic_array_clear_bit(bitmap, index)                                 \
        ((bitmap)[(bitmap_t)((index) / BITS_PER_BITMAP)] &=                    \
         ~(1 << ((index) % BITS_PER_BITMAP)))
#define dynamic_array_test_bit(bitmap, index)                                  \
        (((bitmap)[(bitmap_t)((index) / BITS_PER_BITMAP)] &                    \
          (1 << ((index) % BITS_PER_BITMAP))) != 0)

/**
 * @brief dynamic array의 line에 들어가는 항목에 대한 구조체.
 * 
 */
struct item {
        size_t parent_index;
        key_t key; /**< unique한 item을 찾는 값에 해당한다. */
};

/**
 * @brief 2^index의 item을 가지는 line 구조체이다.
 * 
 */
struct line {
        key_t min; /**< 현재 line에서의 최솟값에 해당한다. */
        key_t max; /**< 현재 line에서의 최댓값에 해당한다. */

        size_t size; /**< 현재 line의 크기를 이야기한다.(2^index) */
        struct item *items; /**< 2^index 만큼의 item이 할당된다. */
};

/**
 * @brief dynamic array에 대한 전체 메타 데이터를 관리하는 배열이다.
 * 
 */
struct dynamic_array {
        size_t nr_lines; /**< dynamic array의 line의 갯수를 지칭한다. */
        size_t size; /**< dynamic array의 현재 크기를 지칭한다. */
        bitmap_t *bitmap; /**< 값이 있는 위치 정보를 가지는 비트맵 */
        struct line *lines; /**< 라인 구조체 */
};

struct dynamic_array *dynamic_array_init(const size_t nr_lines);
int dynamic_array_insert(struct dynamic_array *array, const struct item item);
struct item *dynamic_array_search(struct dynamic_array *array, const key_t key);
int dynamic_array_delete(struct dynamic_array *array, const key_t key);
void dynamic_array_free(struct dynamic_array **_array);

/**
 * @brief dynamic array의 비트맵을 초기화 한다.
 * 
 * @param bitmap 초기화하려고 하는 비트맵의 포인터를 지칭한다.
 * @param nr_lines struct dynamic_array의 nr_lines 값
 * @return bitmap_t* 초기화된 비트맵
 */
static inline bitmap_t *dynamic_array_bitmap_init(const size_t nr_lines)
{
        bitmap_t *bitmap = NULL;
        bitmap = (bitmap_t *)calloc(BITMAP_SIZE(nr_lines), sizeof(bitmap_t));
        if (!bitmap) {
                return NULL;
        }

        return bitmap;
}

/**
 * @brief dynamic array에 있는 bitmap의 할당을 해제한다.
 * 
 * @param bitmap 할당이 되어 있는 비트맵
 * @warning 내부 포인터 값이 변경되니 주의해야 한다.
 */
static inline void dynamic_array_bitmap_free(bitmap_t **_bitmap)
{
        bitmap_t *bitmap = *_bitmap;
        if (bitmap) {
                free(bitmap);
                *_bitmap = NULL;
        }
}

/**
 * @brief dynamic array의 bitmap_idx위치의 bitmap에서 최초로 0이 나오는 비트를 반환한다.
 * 
 * @param bitmap 찾고자 하는 비트맵
 * @param bitmap_idx 현재 탐색 중인 비트맵 인덱스
 * @return size_t 비트맵의 위치. 못 찾은 경우 INDEX_EMPTY가 반환된다.
 */
static inline size_t __dynamic_array_find_first_zero_bit(bitmap_t *bitmap,
                                                         size_t bitmap_idx)
{
        size_t i, pos;
        if (bitmap[bitmap_idx] == (size_t)(~0UL)) {
                return INDEX_EMPTY;
        }

        for (i = 0; i < BITS_PER_BITMAP; i++) {
                pos = bitmap_idx * BITS_PER_BITMAP + i;
                if (!dynamic_array_test_bit(bitmap, pos)) {
                        return pos;
                }
        }

        return INDEX_EMPTY;
}

/**
 * @brief dynamic array의 bitmap에서 최초로 0이 나오는 비트를 반환한다.
 * 
 * @param bitmap 찾고자 하는 비트맵
 * @param nr_lines struct dynamic_array의 nr_lines 값
 * @return size_t 비트맵의 위치. 못 찾은 경우 INDEX_EMPTY가 반환된다.
 */
static inline size_t dynamic_array_find_first_zero_bit(bitmap_t *bitmap,
                                                       const size_t nr_lines)
{
        size_t i, pos;
        for (i = 0; i < BITMAP_SIZE(nr_lines); i++) {
                pos = __dynamic_array_find_first_zero_bit(bitmap, i);
                if (pos != INDEX_EMPTY) {
                        return pos;
                }
        }

        return INDEX_EMPTY;
}

/**
 * @brief dynamic array의 bitmap_idx 위치의 bitmap에서 최초로 1이 나오는 비트를 반환한다.
 * 
 * @param bitmap 찾고자 하는 비트맵
 * @param bitmap_idx 현재 탐색 중인 비트맵 인덱스
 * @return size_t 비트맵의 위치. 못 찾은 경우 INDEX_EMPTY가 반환된다.
 */
static inline size_t __dynamic_array_find_first_bit(bitmap_t *bitmap,
                                                    const size_t bitmap_idx)
{
        size_t i, pos;
        if (bitmap[bitmap_idx] == (size_t)(~0UL)) {
                return INDEX_EMPTY;
        }

        for (i = 0; i < BITS_PER_BITMAP; i++) {
                pos = bitmap_idx * BITS_PER_BITMAP + i;
                if (dynamic_array_test_bit(bitmap, pos)) {
                        return pos;
                }
        }

        return INDEX_EMPTY;
}

/**
 * @brief dynamic array의 bitmap에서 최초로 1이 나오는 비트를 반환한다.
 * 
 * @param bitmap 찾고자 하는 비트맵
 * @param nr_lines struct dynamic_array의 nr_lines 값
 * @return size_t 비트맵의 위치. 못 찾은 경우 INDEX_EMPTY가 반환된다.
 */
static inline size_t dynamic_array_find_first_bit(bitmap_t *bitmap,
                                                  const size_t nr_lines)
{
        size_t i, pos;
        for (i = 0; i < BITMAP_SIZE(nr_lines); i++) {
                pos = __dynamic_array_find_first_bit(bitmap, i);
                if (pos != INDEX_EMPTY) {
                        return pos;
                }
        }

        return INDEX_EMPTY;
}

static inline size_t dynamic_array_get_size(const size_t nitems)
{
        return (log2(nitems + 1) + 1);
}

#endif