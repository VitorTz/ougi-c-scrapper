#include "../../include/structure/introsort.h"


#define INTROSORT_INSERTION_THRESHOLD 24

/* swap, 8 bytes at a time when possible */
static void introsort_swap(char* a, char* b, size_t size) {
    if (a == b) return;
    size_t n = size;
    while (n >= sizeof(uint64_t)) {
        uint64_t tmp;
        memcpy(&tmp, a, sizeof(tmp));
        memcpy(a, b, sizeof(tmp));
        memcpy(b, &tmp, sizeof(tmp));
        a += sizeof(tmp);
        b += sizeof(tmp);
        n -= sizeof(tmp);
    }
    while (n--) {
        char tmp = *a;
        *a++ = *b;
        *b++ = tmp;
    }
}

/* insertion sort: handles small ranges, also finishes every leaf
 * of the quicksort recursion */
static void introsort_insertion(char* base, size_t n, size_t size, SortFunc cmp) {
    if (n < 2) return;
    char tmp[size];
    for (size_t i = 1; i < n; i++) {
        char* cur = base + i * size;
        if (cmp(cur - size, cur) <= 0) continue;
        memcpy(tmp, cur, size);
        char* p = cur;
        do {
            memcpy(p, p - size, size);
            p -= size;
        } while (p > base && cmp(p - size, tmp) > 0);
        memcpy(p, tmp, size);
    }
}

static char* introsort_median3(char* a, char* b, char* c, SortFunc cmp) {
    if (cmp(a, b) < 0) {
        if (cmp(b, c) < 0) return b;
        if (cmp(a, c) < 0) return c;
        return a;
    }
    if (cmp(a, c) < 0) return a;
    if (cmp(b, c) < 0) return c;
    return b;
}

/* heapsort fallback, guarantees O(n log n) worst case */
static void introsort_sift_down(char* base, size_t size, SortFunc cmp, size_t start, size_t end) {
    size_t root = start;
    for (;;) {
        size_t child = root * 2 + 1;
        if (child > end) return;
        if (child + 1 <= end && cmp(base + child * size, base + (child + 1) * size) < 0)
            child++;
        if (cmp(base + root * size, base + child * size) >= 0) return;
        introsort_swap(base + root * size, base + child * size, size);
        root = child;
    }
}

static void introsort_heapsort(char* base, size_t n, size_t size, SortFunc cmp) {
    if (n < 2) return;
    size_t start = (n - 2) / 2;
    while (1) {
        introsort_sift_down(base, size, cmp, start, n - 1);
        if (start == 0) break;
        start--;
    }
    size_t end = n - 1;
    while (end > 0) {
        introsort_swap(base, base + end * size, size);
        end--;
        introsort_sift_down(base, size, cmp, 0, end);
    }
}

static size_t introsort_log2(size_t n) {
    size_t r = 0;
    while (n >>= 1) r++;
    return r;
}

/* quicksort core: Hoare partition, recurse on smaller side, loop
 * on larger side (bounds call stack to O(log n)) */
static void introsort_loop(char* base, size_t n, size_t size, SortFunc cmp, size_t depth_limit) {
    while (n > INTROSORT_INSERTION_THRESHOLD) {
        if (depth_limit == 0) {
            introsort_heapsort(base, n, size, cmp);
            return;
        }
        depth_limit--;

        char* pivot_ptr = introsort_median3(base, base + (n / 2) * size, base + (n - 1) * size, cmp);
        char pivot[size];
        memcpy(pivot, pivot_ptr, size);

        ptrdiff_t i = -1;
        ptrdiff_t j = (ptrdiff_t)n;
        for (;;) {
            do { i++; } while (cmp(base + i * size, pivot) < 0);
            do { j--; } while (cmp(base + j * size, pivot) > 0);
            if (i >= j) break;
            introsort_swap(base + i * size, base + j * size, size);
        }

        size_t left_n = (size_t)(j + 1);
        size_t right_n = n - left_n;

        if (left_n < right_n) {
            introsort_loop(base, left_n, size, cmp, depth_limit);
            base += left_n * size;
            n = right_n;
        } else {
            introsort_loop(base + left_n * size, right_n, size, cmp, depth_limit);
            n = left_n;
        }
    }
    introsort_insertion(base, n, size, cmp);
}

/* public entry point, same signature as qsort */
void introsort(void* base, size_t nmemb, size_t size, SortFunc cmp) {
    if (nmemb < 2 || size == 0) return;
    size_t depth_limit = 2 * introsort_log2(nmemb);
    introsort_loop((char*)base, nmemb, size, cmp, depth_limit);
}