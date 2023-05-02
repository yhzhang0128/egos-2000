/*
 * (C) 2023, Cornell University
 * All rights reserved.
 */

/* Author: Robbert van Renesse
 * Description: course project, user-level threading
 */

#include "app.h"

void ctx_start(void** old_sp, void* new_sp);
void ctx_switch(void** old_sp, void* new_sp);

/* Thread and Semaphore */

struct thread {
    /* Student's code goes here (user-level threading). */
};

void thread_init(){
    /* Student's code goes here (user-level threading). */
}

void thread_create(void (*f)(void *), void *arg, unsigned int stack_size){
    /* Student's code goes here (user-level threading). */
}

void thread_yield(){
    /* Student's code goes here (user-level threading). */
}

void thread_exit(){
    /* Student's code goes here (user-level threading). */
}

void ctx_entry(void){
    /* Student's code goes here (user-level threading). */
}

struct sema {
    /* Student's code goes here (user-level threading). */
};

void sema_init(struct sema *sema, unsigned int count){
    /* Student's code goes here (user-level threading). */
}

void sema_inc(struct sema *sema){
    /* Student's code goes here (user-level threading). */
}

void sema_dec(struct sema *sema){
    /* Student's code goes here (user-level threading). */
}

int sema_release(struct sema *sema){
    /* Student's code goes here (user-level threading). */
}

/* Producer and Consumer */

#define NSLOTS	3

static struct sema s_empty, s_full;
static unsigned int in, out;
static char *slots[NSLOTS];

static void producer(void *arg){
    for (;;) {
        // first make sure there's an empty slot.
        // then add an entry to the queue
        // lastly, signal consumers

        sema_dec(&s_empty);
        slots[in++] = arg;
        if (in == NSLOTS) in = 0;
        sema_inc(&s_full);
    }
}

static void consumer(void *arg){
    for (int i = 0; i < 5; i++) {
        // first make sure there's something in the buffer
        // then grab an entry to the queue
        // lastly, signal producers

        sema_dec(&s_full);
        void *x = slots[out++];
        printf("%s: got '%s'\n", arg, x);
        if (out == NSLOTS) out = 0;
        sema_inc(&s_empty);
    }
}

int main() {
    printf("User-level threading is not implemented.\n");

    /*
    thread_init();
    sema_init(&s_full, 0);
    sema_init(&s_empty, NSLOTS);

    thread_create(consumer, "consumer 1", 16 * 1024);
    thread_create(consumer, "consumer 2", 16 * 1024);
    thread_create(consumer, "consumer 3", 16 * 1024);
    thread_create(consumer, "consumer 4", 16 * 1024);
    thread_create(producer, "producer 2", 16 * 1024);
    thread_create(producer, "producer 3", 16 * 1024);
    producer("producer 1");
    thread_exit();
    */

    return 0;
}

