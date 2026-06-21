#include "../include/arena.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


bool arena_init(ScratchArena* arena, size_t max_capacity) {
    arena->memory = (uint8_t*) malloc(max_capacity);
    
    if (!arena->memory) {
        return false;
    }
    
    arena->capacity = max_capacity;
    arena->used = 0;
    return true;
}

uint8_t* arena_push_block(ScratchArena* arena, const void* source_data, size_t data_size) {
    if (!arena || !source_data || data_size == 0) {
        return NULL;
    }

    if (arena->used + data_size > arena->capacity) {
        fprintf(stderr, "[Error] Arena Buffer Overflow! Cannot push %zu bytes.\n", data_size);
        return NULL;
    }

    uint8_t* destination_ptr = arena->memory + arena->used;
    memcpy(destination_ptr, source_data, data_size);
    arena->used += data_size;
    return destination_ptr;
}


void arena_reset(ScratchArena* arena) {
    arena->used = 0;
}


void arena_free(ScratchArena* arena) {
    if (arena->memory) {
        free(arena->memory);
        arena->memory = NULL;
    }
    arena->capacity = 0;
    arena->used = 0;
}
