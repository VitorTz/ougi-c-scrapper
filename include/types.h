#ifndef TYPES_H
#define TYPES_H
#include <stddef.h>


typedef struct Iterator {
    char* begin;
    char* end;
    char* current;
    size_t step;
} Iterator;


#endif