/**
 * @file dynamic-array.c
 * @author 오기준 (kijunking@pusan.ac.kr)
 * @brief dynamic array의 세부 구현에 대해서 들어가있다.
 * @version 0.1
 * @date 2020-05-18
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "dynamic-array.h"
#include <stdlib.h> // qsort 및 malloc을 위해서 존재
#include <errno.h>
#include <string.h>

/**
 * @brief dynamic array의 특정 인덱스에 해당하는 line을 할당을 받는다.
 * 
 * @param array dynamic array 구조체의 포인터이다.
 * @param line_index 할당 받고자하는 line의 인덱스를 지칭한다.
 * @return int 할당 성공 시에는 0, 아닌 시에는 -N의 값이 반환된다.
 */
static int dynamic_array_line_alloc(struct dynamic_array *array,
                                    size_t line_index)
{
        struct line *line;
        if (line_index >= array->nr_lines) {
                pr_info("line index is over(" SIZE_T_FORMAT "/" SIZE_T_FORMAT
                        ")",
                        line_index, array->nr_lines);
                return -EINVAL;
        }

        line = &array->lines[line_index];
        if (!line->items) {
                line->items =
                        (struct item *)calloc(line->size, sizeof(struct item));
                if (line->items == NULL) {
                        pr_info("Memory allocation failed\n");
                        return -ENOMEM;
                }
        }

        dynamic_array_set_bit(array->bitmap, line_index);
#if defined(TEST) && defined(BIT_COUNTER)
        counter.bit_set_counter[line_index]++;
#endif
#if defined(TEST) && defined(GENERIC_COUNTER)
        counter.alloc++;
#endif

        return 0;
}

/**
 * @brief dynamic array의 특정 인덱스에 해당하는 line을 해제한다.
 * 
 * @param array dynamic array 구조체의 포인터를 지칭한다.
 * @param line_index 해제 대상이 line의 인덱스를 지칭한다.
 * @param flags FREE를 할 지 안 할지를 선택을 하는 플래그이다.
 * @return int 해제 성공 시에는 0, 아닌 시에는 -N의 값이 반환된다.
 */
static int __dynamic_array_line_dealloc(struct dynamic_array *array,
                                        size_t line_index, int flags)
{
        struct line *line;
        if (line_index >= array->nr_lines) {
                pr_info("line index is over(" SIZE_T_FORMAT "/" SIZE_T_FORMAT
                        ")",
                        line_index, array->nr_lines);
                return -EINVAL;
        }

        line = &array->lines[line_index];
        if (flags == FREE_ENABLE && line->items != NULL) {
                free(line->items);
                line->items = NULL;
        }

        line->min = line->max = -1;

        dynamic_array_clear_bit(array->bitmap, line_index);
#if defined(TEST) && defined(BIT_COUNTER)
        counter.bit_unset_counter[line_index]++;
#endif

        return 0;
}

/**
 * @brief dynamic array의 특정 인덱스에 해당하는 line을 해제한다.
 * 
 * @param array dynamic array 구조체의 포인터를 지칭한다.
 * @param line_index 해제 대상이 line의 인덱스를 지칭한다.
 * @return int 해제 성공 시에는 0, 아닌 시에는 -N의 값이 반환된다.
 * 
 * @warning 이 함수를 free를 사용해야 하는 곳에서 사용하지 말 것.
 */
static int dynamic_array_line_dealloc(struct dynamic_array *array,
                                      size_t line_index)
{
#if defined(TEST) && defined(GENERIC_COUNTER)
        counter.dealloc++;
#endif
        return __dynamic_array_line_dealloc(array, line_index, FREE_DISABLE);
}

/**
 * @brief dynamic array에 대한 비교를 수행하는 함수
 * 
 * @param left index가 앞서 있는 내용
 * @param right index가 뒤쳐지는 내용
 * @return int 1: left가 앞서 있음, -1: right가 앞서 있음, 0: 동등함 
 */
static int dynamic_array_compare(const void *left, const void *right)
{
        struct item *item1 = (struct item *)left;
        struct item *item2 = (struct item *)right;

#if defined(TEST) && defined(GENERIC_COUNTER)
        counter.compare++;
#endif
        if (item1->key < item2->key) {
                return -1;
        }

        if (item1->key > item2->key) {
                return 1;
        }

        return 0;
}

/**
 * @brief dynamic array에 값을 삽입하도록 한다.
 * 
 * @param array dynamic array에 대한 포인터를 지칭한다.
 * @param item 삽입하고자 하는 데이터를 지칭한다.
 * @return int 
 */
int dynamic_array_insert(struct dynamic_array *array, const struct item item)
{
        int ret;
        size_t next_pos, line_pos, item_pos, next_item_pos;
        struct line *line = NULL;
        struct line *next_line = NULL;
        struct item *item_ptr = NULL;

        next_item_pos = 0;

        next_pos = dynamic_array_find_first_zero_bit(array->bitmap,
                                                     array->nr_lines);

        ret = dynamic_array_line_alloc(array, next_pos);
        if (ret) {
                goto ret;
        }

        next_line = &array->lines[next_pos];
        for (line_pos = 0; line_pos < next_pos; line_pos++) {
                line = &array->lines[line_pos];
                for (item_pos = 0; item_pos < line->size; item_pos++) {
                        item_ptr = &next_line->items[next_item_pos++];
                        memcpy(item_ptr, &line->items[item_pos],
                               sizeof(struct item));
                        line->items[item_pos].parent_index = line_pos;
                        item_ptr->parent_index = next_pos;
#if defined(TEST) && defined(GENERIC_COUNTER)
                        counter.insert_iter++;
#endif
                } // end of line copy
                dynamic_array_line_dealloc(array, line_pos);
        } // end of previous items copy

        // item copy sequence
        item_ptr = &next_line->items[next_item_pos];
        memcpy(item_ptr, &item, sizeof(struct item));
        item_ptr->parent_index = next_pos;

        qsort(next_line->items, next_line->size, sizeof(struct item),
              dynamic_array_compare);

        next_line->min = next_line->items[0].key;
        next_line->max = next_line->items[next_item_pos].key;
        if (next_pos + 1 > array->size) {
                array->size = next_pos + 1;
        }
ret:
#if defined(TEST) && defined(GENERIC_COUNTER)
        counter.insert++;
#endif
        return ret;
}

/**
 * @brief dynamic array의 탐색 과정에서 binary search를 수행하는 부분
 * 
 * @param line 탐색을 수행할 line
 * @param key 탐색의 대상이 되는 key
 * @return struct item* 탐색 결과 위치. 못 찾은 경우 NULL 반환
 */
static struct item *__dynamic_array_search(struct line *line, const key_t key)
{
        struct item *item = NULL;
        size_t low = 0;
        size_t high = line->size - 1;
        size_t mid;

        while (low <= high) { // binary search operation
                mid = (low + high) / 2;
                item = &line->items[mid];

                if (item->key == key) {
                        return item;
                } else if (item->key > key) {
                        high = mid - 1;
                } else {
                        low = mid + 1;
                }
#if defined(TEST) && defined(GENERIC_COUNTER)
                counter.search_iter++;
#endif
        }
#if defined(TEST) && defined(GENERIC_COUNTER)
        counter.search++;
#endif
        return NULL;
}

/**
 * @brief dynamic array에서 특정 값을 찾도록 한다.
 * 
 * @param array dynamic array의 포인터
 * @param key 찾고자하는 키 값
 * @return struct item* key에 해당하는 위치를 반환한다. 못 찾은 경우 NULL 반환 
 */
struct item *dynamic_array_search(struct dynamic_array *array, const key_t key)
{
        size_t i;
        key_t min, max;
        struct item *item = NULL;
        for (i = 0; i < array->size; i++) {
                min = array->lines[i].min;
                max = array->lines[i].max;
                if (min <= key && key <= max) {
                        item = __dynamic_array_search(&array->lines[i], key);
                        if (item != NULL) {
                                goto ret;
                        } // find key;
                } // min, max check
        } // array search
ret:
        return item;
}

/**
 * @brief dynamic array에서 배제한 item을 제외하고, 지우고자 하는 라인의 item을 밑으로 옮기도록 한다.
 * 
 * @param array dynamic array의 포인터
 * @param nitems 삭제하는 아이템의 갯수
 * @param items 삭제하는 아이템 리스트
 * @param key 삭제에서 제외하고자 하는 key
 * @return int delete 성공 여부(0: 성공)
 */
int __dynamic_array_delete(struct dynamic_array *array, size_t nitems,
                           struct item *items, size_t key)
{
        size_t index;
        int ret = 0;
        for (index = 0; index < nitems; index++) {
                if (items[index].key != key) {
                        ret = dynamic_array_insert(array, items[index]);
                        if (ret) {
                                goto ret;
                        }
                }
        }
ret:
        return ret;
}

/**
 * @brief dynamic array에 대해서 delete를 수행하도록 한다.
 * 
 * @param array dynamic array를 가리키는 포인터
 * @param key dynamic array에서 지우고자 하는 키 값
 * @return int delete 성공 여부(0: 성공)
 */
int dynamic_array_delete(struct dynamic_array *array, const key_t key)
{
        size_t target_index, erase_index;
        struct line *line;
        struct item *items;
        struct item *target_item = dynamic_array_search(array, key);
        int ret = 0;
        if (target_item == NULL) {
                ret = -ENODATA;
                goto ret;
        }

        target_index = target_item->parent_index;
        erase_index =
                dynamic_array_find_first_bit(array->bitmap, array->nr_lines);

        line = &array->lines[erase_index];
        items = array->lines[erase_index].items;
        if (target_index == erase_index) {
                ret = __dynamic_array_delete(array, line->size, items, key);
                if (ret) {
                        goto ret;
                }
                dynamic_array_line_dealloc(array, erase_index);
                if (erase_index == array->size) {
                        array->size--;
                }
        } else {
                ret = __dynamic_array_delete(array, line->size, items,
                                             items[0].key);
                if (ret) {
                        goto ret;
                }
                memcpy(target_item, &items[0], sizeof(struct item));
                target_item->parent_index = target_index;
                dynamic_array_line_dealloc(array, erase_index);
                line = &array->lines[target_index];
                qsort(line->items, line->size, sizeof(struct item),
                      dynamic_array_compare);
                line->min = line->items[0].key;
                line->max = line->items[line->size - 1].key;
        }
ret:
        return ret;
}

/**
 * @brief dynamic array를 초기화 합니다.
 * 
 * @return struct dynamic_array* 초기화된 dynamic array가 반환된다.
 */
struct dynamic_array *dynamic_array_init(const size_t nr_lines)
{
        struct dynamic_array *array;
        size_t line_index;

        array = (struct dynamic_array *)malloc(sizeof(struct dynamic_array));
        if (!array) {
                pr_info("Memory allocation failed\n");
                goto exception;
        }
        array->nr_lines = nr_lines;
        array->size = 0;
        array->bitmap = dynamic_array_bitmap_init(nr_lines);
        if (!array->bitmap) {
                pr_info("Bitmap allocation failed\n");
                goto exception;
        }

        array->lines = (struct line *)malloc(sizeof(struct line) * nr_lines);
        if (!array->lines) {
                pr_info("Line allocation failed\n");
                goto exception;
        }

        for (line_index = 0; line_index < array->nr_lines; line_index++) {
                struct line *line = &array->lines[line_index];
                line->size = (1 << line_index);
                line->items = NULL;
        }
        return array;

exception:
        dynamic_array_free(&array);
        return NULL;
}

/**
 * @brief dynamic array를 해제한다.
 * 
 * @param array 해제하고자 하는 dynamic array
 * @warning array 포인터 값이 변경되므로 주의해야 한다.
 */
void dynamic_array_free(struct dynamic_array **_array)
{
        struct dynamic_array *array = *_array;
        size_t line_index;
        if (!array) {
                pr_info("Nothing to do in free sequence\n");
                return;
        }

        if (array->lines) {
                for (line_index = 0; line_index < array->nr_lines;
                     line_index++) {
                        __dynamic_array_line_dealloc(array, line_index,
                                                     FREE_ENABLE);
                } // end of for
                free(array->lines);
                array->lines = NULL;
        } // array->lines free

        if (array->bitmap) {
                dynamic_array_bitmap_free(&array->bitmap);
        } // array->bitmap free

        free(array);
        *_array = NULL;
}