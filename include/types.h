#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct Iterator {
  size_t step;
  char* begin;
  char* end;
  char* current;
} Iterator;


typedef struct RGB {
  uint8_t r, g, b;
} RGB;


typedef struct {
  size_t bytes;
  uint8_t* data;
  bool success;
} Read;


typedef int (*SortFunc)(const void* a, const void* b);

typedef int (*FilterFunc)(const void* item);

/*
 * Callback signature for converting a single element.
 * The implementation must cast 'src' to the original type, cast 'dest' 
 * to the target type, and perform the assignment.
 */
typedef void (*VectorConvertFunc)(const void* src_item, void* dest_item);


#endif