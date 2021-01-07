/**
 * Algorithms
 *
 * - sorts, both unstable and stable
 */

#ifndef SORT_H
#define SORT_H

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined _WIN32 || defined _WIN64

#define WINDOWS 1
#define restrict __restrict
#define inline __inline
#include <intrin.h> // BitScanReverse
#include <malloc.h> // _malloca

#endif

extern void sort(void *base, size_t nitems, size_t size,
                 int (*comparator)(const void *, const void *));

#endif // SORT_H