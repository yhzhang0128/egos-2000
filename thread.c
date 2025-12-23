#include "hello.c"
#include "thread.h"
#include <sys/queue.h>

/* Student's code goes here (Cooperative Threads). */
/* Define the TCB and helper functions (if needed) for multi-threading. */

/* Student's code ends here. */

void thread_init() {
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
}

void ctx_entry() {
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
}

void thread_create(void (*entry)(void *arg), void *arg, int stack_size) {
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
}

void thread_yield() {
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
}

void thread_exit() {
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
}

/* Student's code goes here (Cooperative Threads). */
/* Define helper functions (if needed) for conditional variables. */

/* Student's code ends here. */

void cv_init(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
}

void cv_wait(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
}

void cv_signal(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
}

#define BUF_SIZE 3
void* buffer[BUF_SIZE];
int count = 0;
int head = 0, tail = 0;
struct cv nonempty, nonfull;

void produce(void* arg) {
    while (1) {
        while (count == BUF_SIZE) cv_wait(&nonfull);
        /* At this point, the buffer is not full. */

        /* Student's code goes here (Cooperative Threads). */
        /* Print out the producer thread ID with the arg pointer. */

        /* Student's code ends here. */
        buffer[tail] = arg;
        tail = (tail + 1) % BUF_SIZE;
        count += 1;
        cv_signal(&nonempty);
    }
}

void consume(void *arg) {
    while (1) {
        while (count == 0) cv_wait(&nonempty);
        /* At this point, the buffer is not empty. */

        /* Student's code goes here (Cooperative Threads). */
        /* Print out the consumer thread ID with the arg pointer. */

        /* Student's code ends here. */
        void* result = buffer[head];
        head = (head + 1) % BUF_SIZE;
        count -= 1;
        cv_signal(&nonfull);
    }
}

int main() {
    thread_init();

    int ID[500];
    for (int i = 0; i < 500; i++) ID[i] = i;

    for (int i = 0; i < 500; i++)
        thread_create(consume, ID + i, STACK_SIZE);

    for (int i = 0; i < 500; i++)
        thread_create(produce, ID + i, STACK_SIZE);

    printf("main thread exits\n\r");
    thread_exit();

    /* The control flow should NOT get here. If the main thread is the last one
     * calling thread_exit(), thread_exit() should then call _end() in thread.s.
     * If the main thread is not the last, thread_exit() will switch the context
     * to another thread. Later, when all threads have called thread_exit(), the
     * last thread calling it should then call _end(). */
}
