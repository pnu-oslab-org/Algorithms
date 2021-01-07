#include <stdio.h>
#include "sort.h"

int cmp(const void *a, const void *b)
{
        return *(int *)a - *(int *)b;
}

int main(void)
{
        int a[] = { 3, 4, 1, 5, 2 };
        sort(a, 5, 4, cmp);
        for (int i = 0; i < 5; i++) {
                printf("%d\n", a[i]);
        }
        return 0;
}