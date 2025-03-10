#include "queue.h"
#define STACK_SIZE   (16 * 1024) // 16 KB stack for children

/* ctx_start and ctx_switch are defined in context.s */
void  ctx_start(void **sp_old, void *sp_new);
void ctx_switch(void **sp_old, void *sp_new);

/* Student's code goes here (Cooperative Threads). */
/* Define data structures for thread control block (TCB) */

/* Student's code ends here. */

struct cv {
/* Student's code goes here (Cooperative Threads). */
/* Define data structures for conditional variables */

/* Student's code ends here. */
};


/**
 * ctx_entry(): Executing in the context of a newly spawned thread. The thread will
 * set up its own state (point its own TCB entry as currently running) and then run
 * its work function held in its TCB entry. Once the thread finishes its work, it will
 * return to this function and exit (by calling thread_exit)
 */
void ctx_entry();

/**
 * thread_init(): Initialize the threading data structures, which includes
 * allocating a TCB entry for the main thread.
 */
void thread_init();

/**
 * thread_create(void (*entry)(void *arg), void *arg, int stack_size): Create a new thread
 * with a [stack_size] byte stack, and schedule it to run.
 *
 * Let T denote the thread that called thread_create.
 * Let T' denote the spawned child thread
 *
 * In thread_create, T will allocate a TCB entry for T' along with T' stack,
 * and then T will schedule T' to run using ctx_start. T' will then call ctx_entry.
 *
 * When some thread T* context switches back to T, T will return back to thread_create,
 * will set itself as the currently running thread, and possibly clean up T* if T* has
 * terminated.
 */
void thread_create(void (*entry)(void *arg), void *arg, int stack_size);

/**
 * thread_yield(): The current thread will schedule another runnable thread, unless
 * there are no other runnable threads, in which the current thread will return
 * back to its work function
 *
 * The thread T that calls thread_yield will schedule T' to run, then run T'
 * using ctx_switch.
 *
 * Once some thread T* context switches back to T, T will return back to thread_yield
 * and cleanup T* if T* has terminated.
 */
void thread_yield();

/**
 * thread_exit(): The current thread will set itself to no longer be scheduled (ZOMBIE),
 * and will then schedule another runnable thread. If no other runnable thread
 * exists, the current thread will exit() (which just infinitely loops).
 *
 * The thread that was scheduled will then free the memory associated with the
 * exiting thread.
 */
void thread_exit();

/**
 * cv_wait(struct cv *condition): Directly analogous to [thread_yield], except
 * that the thread T which called [cv_wait] will wait on the condition variable [cv],
 * and attempt to schedule another thread. If there are no runnable threads, the
 * system is deadlocked and will exit() (which infinitely loops).
 *
 * T cannot be scheduled, and can only be scheduled when another thread T* calls
 * [cv_signal] on the same condition variable. Once T is context switched back to,
 * it will possibly clean up the previous thread.
 */
void cv_wait(struct cv *condition);

/**
 * cv_signal(struct cv *condition): Make at most one thread waiting on [cv] runnable.
 */
void cv_signal(struct cv *condition);
