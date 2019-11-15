///
/// main.c
/// Semaphore
///
/// @author Zeyad Osama.
/// @date 2019-11-16.
/// @copyright © 2019 Zeyad Osama. All rights reserved.
///

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

/**
 * @brief producer threading function.
 */
void *pthread_producer(void *);

/**
 * @brief consumer threading function.
 */
void *pthread_consumer(void *);

int main(void) {
    return 0;
}

void *pthread_producer(void *arg) {
    pthread_exit(NULL);
}

void *pthread_consumer(void *arg) {
    pthread_exit(NULL);
}
