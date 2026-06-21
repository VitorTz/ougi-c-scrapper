#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <pthread.h>
#include <stdbool.h>

/*
 * Custom Semaphore implementation using POSIX threads.
 * Useful for thread synchronization in task queues, thread pools, or ECS systems.
 */
typedef struct {
    int value;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Semaphore;

/* Initializes the semaphore with an initial count */
bool semaphore_init(Semaphore* sem, int initial_value);

/* Destroys the semaphore and frees resources */
void semaphore_destroy(Semaphore* sem);

/* * Decrements the semaphore. 
 * If the value is 0, the calling thread blocks until another thread calls post.
 * Also known as: lock(), acquire(), wait(), down(), P().
 */
void semaphore_wait(Semaphore* sem);

/* * Increments the semaphore and wakes up a blocked thread (if any).
 * Also known as: unlock(), release(), signal(), up(), V().
 */
void semaphore_post(Semaphore* sem);

#endif