#include "../../include/structure/vector.h"
#include <stdlib.h>
#include <string.h>


Vector* vector_create(const size_t data_size, const size_t capacity) {
    Vector* vec = malloc(sizeof(Vector));
    if (vec == NULL) {  return NULL;  }

    vec->data_size = data_size;
    vec->capacity = capacity;
    vec->length = 0;

    if (capacity > 0) {
        vec->ptr = malloc(data_size * capacity);
        if (vec->ptr == NULL) {
            free(vec);
            return NULL;
        }
    } else {
        vec->ptr = NULL;
    }

    return vec;
}

void vector_init(Vector* vec, const size_t data_size, const size_t capacity) {
    if (vec == NULL) {  return ;  }

    vec->data_size = data_size;
    vec->capacity = capacity;
    vec->length = 0;

    if (capacity > 0) {
        vec->ptr = malloc(data_size * capacity);
        if (vec->ptr == NULL) {
            free(vec);
        }
    } else {
        vec->ptr = NULL;
    }    
}

void vector_deinit(Vector* vec) {
    if (vec == NULL) { return; }
    if (vec->ptr != NULL) {
        free(vec->ptr);
    }
}

void vector_destroy(Vector* vec) {
    if (vec == NULL) { return; }
    if (vec->ptr != NULL) {
        free(vec->ptr);
    }
    free(vec);
}


static void vector_grow(Vector* vec) {
    const size_t new_capacity = vec->capacity == 0 ? 1 : vec->capacity * 2;
    void* tmp_ptr = realloc(vec->ptr, new_capacity * vec->data_size);
    
    if (tmp_ptr != NULL) {
        vec->ptr = tmp_ptr;
        vec->capacity = new_capacity;
    }
}


void vector_push_back(Vector* vec, void* src) {
    if (vec->length >= vec->capacity) {
        vector_grow(vec);
    }
    memcpy(
        vec->ptr + (vec->length * vec->data_size), 
        src, 
        vec->data_size
    );
    vec->length++;
}


void* vector_allocate(Vector* vec) {
    if (vec->length >= vec->capacity) {
        vector_grow(vec);
    }
    void* dest = vec->ptr + (vec->length * vec->data_size);
    vec->length++;
    return dest;
}


void vector_insert(Vector* vec, void* src, size_t index) {
    if (index > vec->length) {
        return; 
    }
    
    if (vec->length >= vec->capacity) {
        vector_grow(vec);
    }
    
    if (index < vec->length) {
        const size_t elements_to_move = vec->length - index;        
        memmove(
            vec->ptr + ((index + 1) * vec->data_size),
            vec->ptr + (index * vec->data_size),
            elements_to_move * vec->data_size
        );
    }
    
    memcpy(
        vec->ptr + (index * vec->data_size),
        src,
        vec->data_size
    );
    
    vec->length++;
}


void vector_erase(Vector* vec, size_t index) {
    if (index >= vec->length) { return; }
        
    const size_t elements_to_move = vec->length - index - 1;
    if (elements_to_move > 0) {
        memmove(
            vec->ptr + (index * vec->data_size), 
            vec->ptr + ((index + 1) * vec->data_size), 
            elements_to_move * vec->data_size
        );
    }
    vec->length--;
}


void* vector_at(const Vector* vec, const size_t index) {
    if (index >= vec->length) {
        return NULL;
    }
    return (void*) vec->ptr + (index * vec->data_size);
}


void* vector_pop(Vector* vec, size_t index) {
    if (index >= vec->length) { return NULL; }
    void* tmp = malloc(vec->data_size);
    if (tmp != NULL) {
        memcpy(tmp, vec->ptr + (index * vec->data_size), vec->data_size);
    }
    
    vector_erase(vec, index);
    return tmp;
}


void vector_clear(Vector* vec) {
    vec->length = 0;
}


Iterator vector_iter(const Vector *vec) {
    return (Iterator){
        .begin = vec->ptr,
        .end = vec->ptr + (vec->length * vec->data_size),
        .current = vec->ptr,
        .step = vec->data_size
    };
}


char* vector_iter_next(Iterator* iter) {
    char* tmp = NULL;
    if (iter->current < iter->end) {
        tmp = iter->current;
        iter->current += iter->step;
    }
    return tmp;
}