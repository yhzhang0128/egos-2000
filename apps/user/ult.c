/*
 * (C) 2023, Cornell University
 * All rights reserved.
 */

/* Author: Robbert van Renesse
 * Description: course project, user-level threading
 * Students implement a threading package and semaphore;
 * And then spawn multiple threads as either producer or consumer.
 */

#include "app.h"

/** These two functions are defined in grass/context.S **/
void ctx_start(void** old_sp, void* new_sp);
void ctx_switch(void** old_sp, void* new_sp);

/** Multi-threading functions **/

struct thread {
    /* Student's code goes here. */
};

void thread_init(){
    /* Student's code goes here */
}

void ctx_entry(void){
    /* Student's code goes here. */
}

void thread_create(void (*f)(void *), void *arg, unsigned int stack_size){
    /* Student's code goes here. */
}

void thread_yield(){
    /* Student's code goes here. */
}

void thread_exit(){
    /* Student's code goes here. */
}

/** Semaphore functions **/

struct sema {
    /* Student's code goes here. */
};

void sema_init(struct sema *sema, unsigned int count){
    /* Student's code goes here. */
}

void sema_inc(struct sema *sema){
    /* Student's code goes here. */
}

void sema_dec(struct sema *sema){
    /* Student's code goes here. */
}

int sema_release(struct sema *sema){
    /* Student's code goes here. */
}

/** Producer and consumer functions **/

#define NSLOTS	3

static char *slots[NSLOTS];
static unsigned int in, out;
static struct sema s_empty, s_full;

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
        if (out == NSLOTS) out = 0;
        printf("%s: got '%s'\n", arg, x);
        sema_inc(&s_empty);
    }
}

int main() {
    INFO("User-level threading is not implemented.");

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

