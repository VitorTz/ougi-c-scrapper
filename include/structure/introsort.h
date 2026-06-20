/* introsort.h
 *
 * Drop-in replacement for libc qsort, same call signature.
 * Algorithm: introsort -> quicksort with median-of-3 pivot,
 * insertion-sort cutoff for small ranges, tail-call elimination
 * (recurse on the smaller partition, loop on the larger), and a
 * heapsort fallback when recursion depth exceeds 2*log2(n) to
 * guarantee O(n log n) worst case. Fully in-place, no malloc.
 */

#ifndef INTROSORT_H
#define INTROSORT_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "../types.h"


void introsort(void* base, size_t nmemb, size_t size, SortFunc cmp);


#endif /* INTROSORT_H */

/* ---------------------------------------------------------------
 * Drop-in for your Vector. Include this AFTER your vector.h
 * (needs the Vector struct and SortFunc typedef already visible),
 * or just paste the body into vector.c.
 *
 * void vector_sort(Vector* vec, SortFunc func) {
 *     if (!vec) return;
 *     introsort(vec->ptr, vec->length, vec->data_size, func);
 * }
 * --------------------------------------------------------------- */