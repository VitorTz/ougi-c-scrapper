#include "../include/semaphore.h"


bool semaphore_init(Semaphore* sem, int initial_value) {
    if (!sem || initial_value < 0) {
        return false;
    }

    sem->value = initial_value;

    /* Initialize the mutex for thread-safe access to the 'value' counter */
    if (pthread_mutex_init(&sem->mutex, NULL) != 0) {
        return false;
    }

    /* Initialize the condition variable to handle thread sleeping/waking */
    if (pthread_cond_init(&sem->cond, NULL) != 0) {
        /* Clean up the mutex if condition variable initialization fails */
        pthread_mutex_destroy(&sem->mutex);
        return false;
    }

    return true;
}


void semaphore_destroy(Semaphore* sem) {
    if (!sem) { return; }
    pthread_mutex_destroy(&sem->mutex);
    pthread_cond_destroy(&sem->cond);
}


void semaphore_wait(Semaphore* sem) {
    if (!sem) {
        return;
    }

    /* Lock the mutex before reading or modifying the counter */
    pthread_mutex_lock(&sem->mutex);

    /* * Use a while loop, not an if statement.
     * This protects against "spurious wakeups" (a hardware/OS quirk where 
     * a thread might wake up without a signal). 
     * If it wakes up and the value is still 0, it goes back to sleep.
     */
    while (sem->value <= 0) {
        /* pthread_cond_wait automatically unlocks the mutex while sleeping,
         * and re-locks it immediately upon waking up. */
        pthread_cond_wait(&sem->cond, &sem->mutex);
    }

    /* We successfully acquired the lock, decrement the counter */
    sem->value--;

    /* Release the lock so other threads can evaluate the semaphore */
    pthread_mutex_unlock(&sem->mutex);
}


void semaphore_post(Semaphore* sem) {
    if (!sem) {
        return;
    }

    /* Lock the mutex to safely modify the counter */
    pthread_mutex_lock(&sem->mutex);

    /* Increment the counter */
    sem->value++;

    /* Signal one waiting thread (if any) to wake up and check the condition */
    pthread_cond_signal(&sem->cond);

    /* Release the lock */
    pthread_mutex_unlock(&sem->mutex);
}