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
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <dispatch/dispatch.h>

//#define DEBUG

#define TYPE int

#define DEF_M_CNT 20
#define DEF_B_LEN 4

bool DEBUG = false;

/**
 * @brief queue node structure definition.
 */
typedef struct node {
    TYPE value;
    struct node *next;
} node;

/**
 * @brief queue structure definition.
 */
typedef struct {
    node *front, *rear;
    uint length, max_length;
} queue;

/**
 * @param value to be stored in queue node.
 * @return a queue node ptr with it's default values.
 */
node *init_node(TYPE value);

/**
 * @param length max.
 * @return a queue ptr with it's default values.
 */
queue *init_queue(uint length);

/**
 * @param q to be freed.
 */
int destroy_queue(queue *q);

/**
 * @param q to enqueue in.
 * @param v to be enqueued.
 */
int enqueue(queue *q, TYPE v);

/**
 * @param q to dequeue from.
 */
int dequeue(queue *q);

/**
 * @param q to be checked.
 * @return boolean upoun check.
 */
bool is_empty(queue *q);

/**
 * @param q to be checked.
 * @return boolean upon check.
 */
bool is_full(queue *q);

/**
 * @brief thread's data definition.
 */
typedef struct {
    // running check.
    // for infinite running restriction.
    bool is_running;

    // received message count.
    int cnt;

    // producer/consumer buffer.
    queue *buffer;

    // semaphores.
    dispatch_semaphore_t s_cnt, s_buffer, s_buffer_full, s_buffer_empty;

    // received message indices.
    int t_index;

    // message indices lock.
    pthread_mutex_t m_lock;
} pthread_data_t;

/**
 * @brief producer threading function.
 */
void *pthread_producer(void *);

/**
 * @brief consumer threading function.
 */
void *pthread_consumer(void *);

/**
 * @brief messages threading function.
 */
void *pthread_message(void *);

/**
 * @param buffer_length.
 * @return a pthread_data_t ptr with it's default values.
 */
pthread_data_t *init_args(int buffer_length);

/**
 * @param pthread_data_t to be freed.
 */
void destroy_args(pthread_data_t *args);

/**
 * @brief check running status according to count of useless iterations.
 * @return boolean upon check.
 */
bool pc_check(queue *q);

void print_help();

/// Driver Program.
int main(int argc, char *argv[]) {
    // set default values
    int M_CNT = DEF_M_CNT;
    int B_LENGTH = DEF_B_LEN;

    // terminal proper usage.
    if (argc < 3) {
        print_help();
        fprintf(stderr, "\nwarning: switching to default mode.\n\n");
    } else
        for (int i = 0; i < argc; ++i) {
            if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--message") == 0)
                M_CNT = (int) strtol(argv[++i], (char **) NULL, 10);
            if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--buffer") == 0)
                B_LENGTH = (int) strtol(argv[++i], (char **) NULL, 10);
            if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0)
                DEBUG = true;
        }

    // arguments to be passed between threads.
    // for the sake of communication.
    pthread_data_t *args;
    args = init_args(B_LENGTH);

    // monitor thread creation.
    pthread_t m_monitor_t;
    pthread_create(&m_monitor_t, NULL, pthread_producer, args);

    // collector thread creation.
    pthread_t m_collector_t;
    pthread_create(&m_collector_t, NULL, pthread_consumer, args);

    // messages threads creation.
    pthread_t messages_t[M_CNT];
    for (int i = 0; i < M_CNT; ++i)
        pthread_create(&messages_t[i], NULL, pthread_message, args);

    // threads joining.
    for (int i = 0; i < M_CNT; ++i)
        pthread_join(messages_t[i], NULL);
    pthread_join(m_monitor_t, NULL);
    pthread_join(m_collector_t, NULL);

    destroy_args(args);
    return EXIT_SUCCESS;
}

void *pthread_producer(void *arg) {
    pthread_data_t *arg_t = (pthread_data_t *) arg;
    int cnt;
    while (arg_t->is_running) {
        // random sleep.
        sleep(random() % 10 + 1);

        printf("monitor thread:: waiting to read counter.\n");
        dispatch_semaphore_wait(arg_t->s_cnt, DISPATCH_TIME_FOREVER);
        cnt = arg_t->cnt;
        arg_t->cnt = 0;
        dispatch_semaphore_signal(arg_t->s_cnt);
        printf("monitor thread:: reading a count value of %d succeeded.\n", cnt);

        if (is_full(arg_t->buffer))
            printf("monitor thread:: buffer is full.\n");

        // wait upon full buffer.
        dispatch_semaphore_wait(arg_t->s_buffer_full, DISPATCH_TIME_FOREVER);
        dispatch_semaphore_wait(arg_t->s_buffer, DISPATCH_TIME_FOREVER);
        int len = enqueue(arg_t->buffer, cnt);
        dispatch_semaphore_signal(arg_t->s_buffer);
        dispatch_semaphore_signal(arg_t->s_buffer_empty);
        printf("monitor thread:: writing to buffer at position %d succeeded.\n", len);
    }
    fprintf(stderr, "warning:: producer thread has been activating for so long without any change.\n"
                    "warning:: producer thread will terminate.\n");
    pthread_exit(NULL);
}

void *pthread_consumer(void *arg) {
    pthread_data_t *arg_t = (pthread_data_t *) arg;
    queue q_condition;
    q_condition.max_length = INT8_MAX;
    int val;
    while (true) {
        // random sleep.
        sleep(random() % 10 + 1);

        if (is_empty(arg_t->buffer))
            printf("collector thread:: buffer is empty.\n");

        // wait upon empty buffer.
        dispatch_semaphore_wait(arg_t->s_buffer_empty, DISPATCH_TIME_FOREVER);
        dispatch_semaphore_wait(arg_t->s_buffer, DISPATCH_TIME_FOREVER);
        val = dequeue(arg_t->buffer);
        printf("collector thread:: reading value %d from buffer at position %d succeeded.\n", val,
               arg_t->buffer->length);
        dispatch_semaphore_signal(arg_t->s_buffer);
        dispatch_semaphore_signal(arg_t->s_buffer_full);

        // termination check.
        enqueue(&q_condition, val);
        if (pc_check(&q_condition)) {
            arg_t->is_running = false;
            break;
        }
    }
    fprintf(stderr, "warning:: consumer thread has been activating for so long without any change.\n"
                    "warning:: consumer thread will terminate.\n");
    pthread_exit(NULL);
}


void *pthread_message(void *arg) {
    pthread_data_t *arg_t = (pthread_data_t *) arg;

    pthread_mutex_lock(&arg_t->m_lock);
    int i = ++arg_t->t_index;
    pthread_mutex_unlock(&arg_t->m_lock);

    long time = random() % 10 + 1;
    sleep(time);
    printf("counter thread %02d:: message received.\n", i);
    sleep(time);
    printf("counter thread %02d:: waiting to write.\n", i);

    dispatch_semaphore_wait(arg_t->s_cnt, DISPATCH_TIME_FOREVER);
    printf("counter thread %02d:: adding to counter.\n", i);
    printf("counter thread %02d:: counter:: value = %d\n", i, ++arg_t->cnt);
    dispatch_semaphore_signal(arg_t->s_cnt);

    pthread_exit(NULL);
}

bool pc_check(queue *q) {
    int cnt = 0, length = 0;
    node *n = q->front;
    while (n != NULL) {
        if (length < q->length - 3 && n->value == 0)
            cnt++;
        length++;
        n = n->next;
    }
    return cnt >= 2;
}

pthread_data_t *init_args(int buffer_length) {
    pthread_data_t *args = malloc(sizeof(pthread_data_t));
    if (args == NULL) {
        perror("args:: ");
        exit(EXIT_FAILURE);
    }
    args->buffer = init_queue(buffer_length);
    if (args->buffer == NULL) {
        perror("buffer:: ");
        exit(EXIT_FAILURE);
    }
    args->cnt = 0;
    args->t_index = 0;
    args->is_running = true;
    args->s_cnt = dispatch_semaphore_create(1);
    args->s_buffer = dispatch_semaphore_create(1);
    args->s_buffer_full = dispatch_semaphore_create(buffer_length);
    args->s_buffer_empty = dispatch_semaphore_create(0);
    pthread_mutex_init(&args->m_lock, NULL);

    return args;
}

void destroy_args(pthread_data_t *args) {
    destroy_queue(args->buffer);
    pthread_mutex_destroy(&args->m_lock);
    free(args);
}

node *init_node(TYPE value) {
    node *n = malloc(sizeof(node));
    if (n == NULL) {
        perror("node:: ");
        exit(EXIT_FAILURE);
    }
    n->value = value;
    n->next = NULL;
    return n;
}

queue *init_queue(uint length) {
    queue *q = (queue *) malloc(sizeof(queue));
    if (q == NULL) {
        perror("queue:: ");
        exit(EXIT_FAILURE);
    }
    q->front = NULL;
    q->rear = NULL;
    q->length = 0;
    q->max_length = length;
    if (DEBUG)
        printf("buffer:: %p:: initialization successful.\n", &q);
    return q;
}

int destroy_queue(queue *q) {
    while (!is_empty(q))
        dequeue(q);
    free(q);
    if (DEBUG)
        printf("buffer:: %p:: destroyed successful.\n", &q);
    return 0;
}

int enqueue(queue *q, TYPE value) {
    if (DEBUG)
        printf("buffer:: %p:: enqueue.\n", &q);

    if (q->length >= q->max_length)
        return -1;
    node *node = init_node(value);
    struct node *rear = q->rear;
    q->rear = node;
    if (rear == NULL)
        q->front = node;
    else
        rear->next = node;
    q->length++;

    if (DEBUG)
        printf("buffer:: %p:: length:: %d.\n", &q, q->length);

    return q->length;
}

int dequeue(queue *q) {
    if (DEBUG)
        printf("buffer:: %p:: dequeue.\n", &q);

    if (!is_empty(q)) {
        node *temp = q->front;
        TYPE value = temp->value;
        q->front = temp->next;
        free(temp);
        if (is_empty(q))
            q->rear = NULL;
        q->length--;

        if (DEBUG)
            printf("buffer:: %p:: length:: %d.\n", &q, q->length);

        return value;
    }
    return INT32_MIN;
}

bool is_empty(queue *q) {
    return q->front == NULL ? true : false;
}

bool is_full(queue *q) {
    return q->length >= q->max_length ? true : false;
}

void print_help() {
    printf("Usage:\n"
           "\t./sem -m <messages-count>\n"
           "\t./sem -b <buffer-size>\n"
           "\t./sem -m <messages-count> -b <buffer-size>\n"
           "\t./sem -d -m <messages-count> -b <buffer-size>\n"

           "\nVerbose:\n"
           "\t--message\t\t<messages-count>.\n"
           "\t--buffer\t\t<buffer-size>.\n"
           "\t--debug\t\t<debug-mode>.\n"

           "\nOptions:\n"
           "\t-d\t\t\t\tDebug mode.\n");
}
