#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>
#include <stddef.h>


typedef struct Iterator {
    char* begin;
    char* end;
    char* current;
    size_t step;
} Iterator;


typedef struct RGB {
  uint8_t r, g, b;
} RGB;


typedef int (*SortFunc)(const void* a, const void* b);

typedef int (*FilterFunc)(const void* item);


#endif