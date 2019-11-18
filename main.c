///
/// main.c
/// Semaphore
///
/// @author Zeyad Osama.
/// @date 2019-11-16.
/// @copyright © 2019 Zeyad Osama. All rights reserved.
///

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#define DEBUG

#define TYPE int
#define BUFFER_LENGTH 10

const char *ERROR = "error";

typedef struct node {
    TYPE value;
    struct node *next;
} node;

typedef struct {
    node *front;
    node *rear;
    int max_length;
    int length;
} queue;

node *init_node(TYPE value);

int init_queue(queue *q, uint length);

int destroy_queue(queue *q);

int enqueue(queue *q, TYPE v);

int dequeue(queue *q);

bool is_empty(queue *q);

bool is_full(queue *q);

/**
 * @brief thread's data definition.
 */
typedef struct {
    int cnt;
    queue *buffer;
    pthread_mutex_t lock;
} pthread_data_t;

/**
 * @brief producer threading function.
 */
void *pthread_producer(void *);

/**
 * @brief consumer threading function.
 */
void *pthread_consumer(void *);

void init_args(pthread_data_t *args);

void destroy_args(pthread_data_t *args);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "USAGE:./main.out <INT> <INT> <INT>\n");
    }

    pthread_data_t args;
    init_args(&args);

    int n = 100;
    pthread_t m_monitor[n];
    for (int i = 0; i < n; ++i)
        pthread_create(&m_monitor[i], NULL, pthread_producer, &args);

    // threads joining.
    for (int i = 0; i < n; ++i)
        pthread_join(m_monitor[i], NULL);

    printf("%d\n", args.cnt);

    destroy_args(&args);
    return 0;
}

void *pthread_producer(void *arg) {
    pthread_data_t *t_args = arg;
    pthread_mutex_lock(&(t_args->lock));
    printf("counter thread %d:: received a message.\n", t_args->cnt);
    t_args->cnt += 1;
    pthread_mutex_unlock(&(t_args->lock));
    pthread_exit(NULL);
}

void *pthread_consumer(void *arg) {
    pthread_exit(NULL);
}

void init_args(pthread_data_t *args) {
    args->cnt = 0;
    init_queue((args->buffer), BUFFER_LENGTH);
    pthread_mutex_init(&(args->lock), NULL);
}

void destroy_args(pthread_data_t *args) {
    destroy_queue(args->buffer);
    pthread_mutex_destroy(&(args->lock));
}

node *init_node(TYPE value) {
    node *n = malloc(sizeof(n));
    if (n == NULL)
        return NULL;
    n->value = value;
    n->next = NULL;
    return n;
}

int init_queue(queue *q, uint length) {
    q = malloc(sizeof(q));
    if (q == NULL)
        return -1;
    q->front = NULL;
    q->rear = NULL;
    q->length = 0;
    q->max_length = length;
    return 0;
}

int destroy_queue(queue *q) {
    while (!is_empty(q))
        dequeue(q);
    free(q);
    return 0;
}

int enqueue(queue *q, TYPE v) {
    if (q->length >= q->max_length)
        return -1;
    node *node = init_node(v);
    struct node *rear = q->rear;
    q->rear = node;
    if (rear == NULL)
        q->front = node;
    else
        rear->next = node;
    q->length++;
    return 0;
}

int dequeue(queue *q) {
    if (!is_empty(q)) {
        node *temp = q->front;
        TYPE value = temp->value;
        q->front = temp->next;
        free(temp);
        if (is_empty(q))
            q->rear = NULL;
        q->length--;
        return value;
    }
    return -1;
}

bool is_empty(queue *q) {
    return q->front == NULL ? true : false;
}

bool is_full(queue *q) {
    return q->length >= q->max_length ? true : false;
}