#include "../include/hash.h"
#include <stdint.h>


// FNV-1a hash constants depend on the size of the target type (size_t).
// These preprocessor directives ensure the correct magic numbers are used 
// whether compiling for a 32-bit or 64-bit architecture.
#if SIZE_MAX == UINT64_MAX
    #define FNV_OFFSET_BASIS 14695981039346656037ULL
    #define FNV_PRIME 1099511628211ULL
#else
    #define FNV_OFFSET_BASIS 2166136261U
    #define FNV_PRIME 16777619U
#endif

/**
 * Computes the FNV-1a hash of a null-terminated string.
 * * @param str The input string to be hashed.
 * @return The computed size_t hash value.
 */
size_t hash_string(const void* __src) {
    const char* str = (const char*) __src;
    if (str == NULL) { return 0; }

    size_t hash = FNV_OFFSET_BASIS;

    while (*str != '\0') {
        hash ^= (size_t)(unsigned char)(*str);
        hash *= FNV_PRIME;        
        str++;
    }

    return hash;
}