#ifndef ARENA_H
#define ARENA_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Define a reusable memory arena for single-threaded processing
typedef struct {
    uint8_t* memory;
    size_t capacity;
    size_t used;
} ScratchArena;

// Initialize the arena with a massive fixed size (e.g., 100MB)
bool arena_init(ScratchArena* arena, size_t max_capacity);

// Reset the "used" counter instead of freeing memory
// This makes the arena ready for the next image instantly
void arena_reset(ScratchArena* arena);

// Free the actual memory only when the program is closing
void arena_free(ScratchArena* arena);

/* Copies a raw block of memory into the arena and updates the used marker */
uint8_t* arena_push_block(ScratchArena* arena, const void* source_data, size_t data_size);

#endif