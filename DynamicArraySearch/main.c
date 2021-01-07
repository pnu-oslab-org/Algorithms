#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dynamic-array.h"

// #define DELETE_TEST

#ifdef DELETE_TEST
#define SIZE 10
#endif

int main(void)
{
        int ret = 0;
        size_t nr_case = 0;
#ifndef DELETE_TEST
        size_t i = 0;
#endif
        struct dynamic_array *array = NULL;
#ifndef DELETE_TEST
        size_t err = 0, total = 0;
#endif
        size_t array_size = 0;

#ifdef TEST
        memset(&counter, 0, sizeof(struct counter));
#endif

        FILE *fp = fopen("test.inp", "r");

        fscanf(fp, SIZE_T_FORMAT, &nr_case);

        array_size = dynamic_array_get_size(nr_case);
        array = dynamic_array_init(array_size);
        if (array == NULL) {
                goto exception;
        }
        pr_info("dynamic array initialize finished\n");
#if defined(TEST) && defined(BIT_COUNTER)
        counter.size = array->size;
        counter.bit_set_counter = (int *)calloc(nr_case, sizeof(int));
        counter.bit_unset_counter = (int *)calloc(nr_case, sizeof(int));
#endif

#ifdef DELETE_TEST
        for (int i = 0; i < SIZE; i++) {
                struct item item = { .key = i };
                dynamic_array_insert(array, item);
        }
        pr_info("");
        for (int i = 0; i < SIZE; i++) {
                struct item *item = dynamic_array_search(array, i);
                if (item) {
                        printf("%d(%d) ", (int)item->key,
                               (int)item->parent_index);
                } else {
                        printf("NULL ");
                }
        }
        printf("\n");
        for (int j = SIZE - 1; j >= 0; j--) {
                // for (int j = 0; j < SIZE; j++) {
                pr_info("");
                printf("================\n");

                dynamic_array_delete(array, j);
                pr_info("");
                for (int i = 0; i < SIZE; i++) {
                        struct item *item = dynamic_array_search(array, i);
                        if (item) {
                                printf("%d(%d) ", (int)item->key,
                                       (int)item->parent_index);
                        } else {
                                printf("NULL ");
                        }
                }
                printf("\n");
        }
#else

#ifdef TEST
        PRINT_HEADER();
#endif
        for (i = 0; i < nr_case; i++) {
                char command[256];
                key_t key;
#ifdef TEST
#ifdef BIT_COUNTER
                counter.size =
                        array->size > counter.size ? array->size : counter.size;
#endif
                if (i % 1000 == 0) {
                        PRINT_COUNTER();
                }
#endif
                fscanf(fp, "%s " KEY_FORMAT, command, &key);
                if (!strncmp(command, "INSERT", sizeof(command))) {
                        struct item item = { .key = key };
                        ret = dynamic_array_insert(array, item);
                        if (ret) {
                                goto exception;
                        }
                }
                if (!strncmp(command, "SEARCH", sizeof(command))) {
                        struct item *item = dynamic_array_search(array, key);
                        if (item == NULL) {
                                err++;
                        }
                        total++;
                }
        }
#ifdef TEST
        PRINT_COUNTER();
#endif
        pr_info("insert/serach sequence finished (err: %.2f%%)\n",
                (float)(err * 100.0 / total));
#endif

exception:
#if defined(TEST) && defined(BIT_COUNTER)
        free(counter.bit_set_counter);
        free(counter.bit_unset_counter);
#endif
        if (array) {
                dynamic_array_free(&array);
                pr_info("dynamic array free\n");
        }
        fclose(fp);
        return ret;
}