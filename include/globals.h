#ifndef GLOBALS_H
#define GLOBALS_H
#include <stdbit.h>
#include "semaphore.h"
#include "arena.h"


void globals_init();

void globals_close();

Semaphore* globals_get_sem_image_util();

Semaphore* globals_get_sem_image_download();

ScratchArena* globals_get_arena(bool reset);


#endif