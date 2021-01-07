#include "van-emde-boas.h"
#ifdef __cplusplus
#include <cstdio>
#include <bitset>
#include <ctime>
#else
#include <stdio.h>
#include <time.h>
#endif

#define FILE_NAME "test"
#define COMMAND_SIZE 256

#if defined(__cplusplus) && defined(BITSET)
#endif

int main(void)
{
        FILE *fin = NULL;
        FILE *fout = NULL;
        int nr_test_case, size, i;
        double start_time, end_time;

#if defined(__cplusplus) && defined(BITSET)
        std::bitset<(1 << 23)> bits;
        bits.reset();

        printf("bitset case \n");
#else
        struct vEB *V = NULL;

        printf("van emde boas case \n");
#endif

        fin = fopen(FILE_NAME ".inp", "r");
        if (fin == NULL) {
                pr_info("file read failed (%s)\n", FILE_NAME ".inp");
                goto exception;
        }

        fout = fopen(FILE_NAME ".out", "w");
        if (fout == NULL) {
                pr_info("file write failed (%s)\n", FILE_NAME ".out");
                goto exception;
        }

        fscanf(fin, "%d %d", &nr_test_case, &size);
#if !(defined(__cplusplus) && defined(BITSET))
        V = vEB_init(size + 1);
        if (V == NULL) {
                goto exception;
        }
#endif
        start_time = ((double)clock() / CLOCKS_PER_SEC);
        for (i = 0; i < nr_test_case; i++) {
                char command[COMMAND_SIZE];
                int index;
                fscanf(fin, "%s %d", command, &index);
                if (i % (int)(nr_test_case / 10) == 0) {
                        end_time = ((double)clock() / CLOCKS_PER_SEC);
                        printf("%lf\n", end_time - start_time);
                }
                if (!strncmp(command, "INSERT", sizeof(command))) {
#if defined(__cplusplus) && defined(BITSET)
                        bits.set(index);
#else
                        vEB_tree_insert(V, index);
#endif
                } else if (!strncmp(command, "SEARCH", sizeof(command))) {
                        bool is_exist = false;
                        int pre = NIL, suc = NIL;
#if defined(__cplusplus) && defined(BITSET)
                        is_exist = bits.test(index);
#else
                        is_exist = vEB_tree_member(V, index);
#endif
#if defined(__cplusplus) && defined(BITSET)
                        int i;
                        for (i = index + 1; i < size; i++) {
                                if (bits.test(i)) {
                                        suc = i;
                                        break;
                                }
                        }

                        for (i = index - 1; i >= 0; i--) {
                                if (bits.test(i)) {
                                        pre = i;
                                        break;
                                }
                        }
#else
                        pre = vEB_tree_predecessor(V, index);
                        suc = vEB_tree_successor(V, index);
#endif
                        fprintf(fout, "%d %d %d\n", is_exist, pre, suc);
                } else if (!strncmp(command, "REMOVE", sizeof(command))) {
#if defined(__cplusplus) && defined(BITSET)
                        bits.reset(index);
#else
                        vEB_tree_delete(V, index);
#endif
                } else {
                        pr_info("invalid command detected (%s)\n", command);
                        goto exception;
                }
        }
        end_time = ((double)clock() / CLOCKS_PER_SEC);
        printf("%lf\n", end_time - start_time);
#if !(defined(__cplusplus) && defined(BITSET))
        vEB_free(V);
#endif
        fclose(fin);
        fclose(fout);
        return 0;

exception:
        if (fin) {
                fclose(fin);
        }
        if (fout) {
                fclose(fout);
        }
#if !(defined(__cplusplus) && defined(BITSET))
        if (V) {
                vEB_free(V);
        }
#endif
        return -1;
}

#if defined(__cplusplus) && defined(BITSET)
#endif