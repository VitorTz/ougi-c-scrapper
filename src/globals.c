#include "../include/globals.h"
#include "../include/constants.h"


static Semaphore sem_image_util = {0};
static Semaphore sem_image_download = {0};
static ScratchArena arena = {0};


void globals_init() {
    semaphore_init(&sem_image_util, 8);
    semaphore_init(&sem_image_download, 8);
    arena_init(&arena, IMAGE_MAX_SIZE_IN_BYTES);
}


void globals_close() {
    semaphore_destroy(&sem_image_util);
    semaphore_destroy(&sem_image_download);
    arena_free(&arena);
}


Semaphore* globals_get_sem_image_util() {
    return &sem_image_util;
}


Semaphore* globals_get_sem_image_download() {
    return &sem_image_download;
}


ScratchArena* globals_get_arena(const bool reset) {
    if (reset) { arena_reset(&arena); }
    return &arena;
}