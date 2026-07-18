/* Student's code goes here (Cooperative Threads). */
enum thread_status {
	THREAD_RUNNING,
    /* Define the various possible status of a thread. */

};

struct thread {
    int id;
    void* sp;
    enum thread_status status;
    /* Define the data structure for thread control block. */

};

struct cv {
    /* Define the data structure for conditional variables. */

};
/* Student's code ends here. */




/* Every thread created by thread_create() has a 1024-byte
   stack. When creating 1000 threads, 1000*1KB≈1MB will be
   allocated from the *heap* as the stack of these threads. */
#define STACK_SIZE 1024

/* ctx_start() and ctx_switch() are defined in context.s;
   They are called by thread_create(), etc., in thread.c. */
void ctx_start(void **sp_old, void *sp_new);
void ctx_switch(void **sp_old, void *sp_new);

/* _end() is defined in thread.s. */
void _end();

/**
 * ctx_entry(): Executing in the context of a newly created thread. The thread
 * sets up its own state (e.g., point its own TCB entry as currently running),
 * and call its entry function held in its TCB entry. Once the entry function
 * returns to ctx_entry(), ctx_entry() should call thread_exit().
 */
void ctx_entry();

/**
 * thread_init(): Initialize the data structures for multi-threading, such as
 * allocating a TCB entry for the main thread.
 */
void thread_init();

/**
 * thread_create(void (*entry)(void *arg), void *arg): Create a new thread with
 * a stack of STACK_SIZE (defined above) bytes, and schedule it to run.
 *
 * Let T denote the thread that called thread_create().
 * Let T' denote the newly created thread.
 *
 * In thread_create(), T will allocate a TCB entry for T' along with the stack
 * of T', and then T will schedule T' to run by calling ctx_start().
 *
 * When some thread context switches back to T, T returns from ctx_switch() to
 * thread_create() as if ctx_start() returns. See thread.s for the details.
 *
 * After ctx_start() returns, the current thread should check if the previously
 * running thread has terminated. If so, it should free the stack and TCB of the
 * previously running thread.
 */
void thread_create(void (*entry)(void *arg), void *arg);

/**
 * thread_yield(): Switch to another thread by calling ctx_switch(). If no other
 * thread can be switched to, continue to run the current thread (if it has not
 * exited or been waiting on a semaphore).
 *
 * After ctx_switch() returns, the current thread should check if the previously
 * running thread has terminated. If so, it should free the stack and TCB of the
 * previously running thread.
 */
void thread_yield();

/**
 * thread_exit(): The current thread will set its status to TERMINATED (or call
 * it ZOMBIE), and yield by calling thread_yield(). If thread_yield() finds that
 * all other threads have terminated, it should call the _end() in thread.s.
 */
void thread_exit();

/**
 * cv_init(struct cv *condition): Initialize the fields in struct cv.
 */
void cv_init(struct cv *condition);

/**
 * cv_wait(struct cv *condition): Remove the current thread from the TCB, and
 * add it to the conditional variable. Try to yield to another thread.
 */
void cv_wait(struct cv *condition);

/**
 * cv_signal(struct cv *condition): Remove a waiting thread (if exists) from the
 * conditional variable, and add it back to the TCB so that it can be scheduled.
 * However, cv_signal() should not switch the context to another thread (i.e.,
 * the current thread should continue to run).
 */
void cv_signal(struct cv *condition);