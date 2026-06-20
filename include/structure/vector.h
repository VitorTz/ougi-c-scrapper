#ifndef VECTOR_H
#define VECTOR_H
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include "../types.h"


typedef struct vector {
    char* ptr;
    size_t data_size;
    size_t length;
    size_t capacity;
} Vector;


Vector* vector_create(size_t data_size, size_t capacity);

void vector_init(Vector* vec, size_t data_size, size_t capacity);

void* vector_data(Vector* vec);

void vector_deinit(Vector* vec);

void vector_reserve(Vector* vec, size_t capacity);

void vector_push_back(Vector* vec, const void* data);

void* vector_allocate(Vector* vec);

bool vector_assign(Vector* vec, const void* src, size_t num_items);

void vector_insert(Vector* vec, void* data, size_t index);

void vector_erase(Vector* vec, size_t index);

void* vector_at(const Vector* vec, size_t index);

void* vector_pop(Vector* vec, size_t index);

void vector_destroy(Vector* vec);

Iterator vector_iter(const Vector* vec);

void vector_clear(Vector* vec);

size_t vector_size(const Vector* vec);

bool vector_is_empty(const Vector* vec);

char* vector_iter_next(Iterator* iter);


/* * Generic foreach macro for Vector iteration.
 * * Parameters:
 * type   - The data type being cast (e.g., Path)
 * it_ptr - The pointer variable name to hold the current item
 * vec    - The Vector pointer being iterated over
 */
#define VECTOR_FOREACH(type, it_ptr, vec) \
    for (Iterator _iter_##it_ptr = vector_iter(vec); \
         ((it_ptr) = (type*) vector_iter_next(&_iter_##it_ptr)) != NULL;)


#endif