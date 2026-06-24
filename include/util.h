#ifndef UTIL_H
#define UTIL_H
#include <stdbool.h>
#include "structure/string_t.h"
#include "structure/vector.h"
#include "structure/path.h"

/* Safe min/max macros using compiler extensions (GCC/Clang).
 * Variables are evaluated only once, preventing side-effect bugs 
 * when passing expressions like i++ or function calls. */
#define MIN(a, b) \
    ({ \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b; \
    })

#define MAX(a, b) \
    ({ \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b; \
    })

/*
 * Parses the primary and secondary numbers from the filename.
 * Expected format: "path/to/file/XX_Y.webp" -> extracts XX and Y.
 */
void extract_file_numbers(const char* filepath, int* primary, int* secondary);


/*
 * Comparison function for qsort to sort CString items by their filename numbers.
 * Sorts ascendingly by the primary number, then by the secondary number.
 */
bool compare_numbered_filenames(const void* a, const void* b);


void sleep_ms(int milliseconds);


    
#endif