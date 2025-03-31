#include "queue.h"
#define STACK_SIZE   (16 * 1024) // 16 KB stack for children

/* Student's code goes here (Cooperative Threads). */
enum thread_status {
	THREAD_RUNNING,
/* Define possible status of a thread */

};

struct thread {
    int id;
    void* sp;
    enum thread_status status;
/* Define the data structure for thread control block (TCB) */

};
/* Student's code ends here. */

/**
 * ctx_entry(): Executing in the context of a newly spawned thread. The thread will
 * set up its own state (e.g., point its own TCB entry as currently running) and then
 * run its entry function held in its TCB entry. Once the thread finishes its work, it
 * will return to this function and exit (by calling thread_exit).
 */
void ctx_entry();

/**
 * thread_init(): Initialize the data structures for multi-threading, which includes
 * allocating a TCB entry for the main thread.
 */
void thread_init();

/**
 * thread_create(void (*entry)(void *arg), void *arg, int stack_size): Create a new thread
 * with a [stack_size] byte stack, and schedule it to run.
 *
 * Let T denote the thread that called thread_create.
 * Let T' denote the spawned child thread.
 *
 * In thread_create, T will allocate a TCB entry for T' along with T' stack,
 * and then T will schedule T' to run using ctx_start.
 *
 * When some thread T'' context switches back to T, T will return back to thread_create,
 * and possibly clean up T'' if T'' has terminated.
 */
void thread_create(void (*entry)(void *arg), void *arg, int stack_size);

/**
 * thread_yield(): Try to switch to another thread using ctx_switch. If no other thread
 * can be switched to, continue to run the current thread (if it is still runnable).
 *
 * Once some thread T' context switches back to T, T will return back to thread_yield
 * and cleanup T' if T' has terminated.
 */
void thread_yield();

/**
 * thread_exit(): The current thread will set its status ZOMBIE (cannot be scheduled),
 * and yield to another runnable thread. If no other runnable thread exists, call the
 * _end() in thread.s which just infinitely loops.
 */
void thread_exit();

/* Student's code goes here (Cooperative Threads). */
struct cv {
/* Define data structures for conditional variables */

};
/* Student's code ends here. */

/**
 * cv_wait(struct cv *condition): Remove the current thread from the TCB and add it
 * to the conditional variable. Try to yield to a runnable thread in the TCB.
 */
void cv_wait(struct cv *condition);

/**
 * cv_signal(struct cv *condition): Remove a thread (if exists) from the conditional
 * variable and add it back to the TCB so that it can be scheduled later.
 */
void cv_signal(struct cv *condition);

/* ctx_start and ctx_switch are defined in context.s */
void  ctx_start(void **sp_old, void *sp_new);
void ctx_switch(void **sp_old, void *sp_new);

/* _end is defined in thread.s */
void _end();