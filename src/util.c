#include "../include/structure/string_t.h"
#include "../include/util.h"
#include <string.h>
#include <time.h>
#include <stdio.h>


static char buffer[256];


/*
 * Parses the primary and secondary numbers from the filename.
 * Expected format: "path/to/file/XX_Y.webp" -> extracts XX and Y.
 */
void extract_file_numbers(const char* filepath, int* primary, int* secondary) {
    *primary = 0;
    *secondary = 0;
    
    if (!filepath) return;

    /* Find the last directory separator to isolate the filename portion */
    const char* filename = strrchr(filepath, '/');
    if (filename) {
        filename++; /* Move past the '/' character */
    } else {
        filename = filepath; /* Fallback if no '/' is present */
    }

    /* Extract the integers formatted as "%d_%d" */
    sscanf(filename, "%d_%d", primary, secondary);
}

/*
 * Comparison function for qsort to sort CString items by their filename numbers.
 * Sorts ascendingly by the primary number, then by the secondary number.
 */
bool compare_numbered_filenames(const void* a, const void* b) {
    /* * Since the Vector stores CString structs by value, the pointers 
     * passed by qsort point directly to the CString elements.
     */
    const string_t* str_a = (const string_t*) a;
    const string_t* str_b = (const string_t*) b;

    int primary_a = 0, secondary_a = 0;
    int primary_b = 0, secondary_b = 0;

    /* Extract the numbers from both CString data buffers */
    extract_file_numbers(str_a->data, &primary_a, &secondary_a);
    extract_file_numbers(str_b->data, &primary_b, &secondary_b);

    /* Primary sort: compare the first number (e.g., the '2' vs '10') */
    if (primary_a != primary_b) {
        return primary_a - primary_b;
    }

    /* Secondary sort: if primary numbers are equal, compare the sub-number */
    return secondary_a - secondary_b;
}


void sleep_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}
