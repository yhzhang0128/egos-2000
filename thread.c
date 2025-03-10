void terminal_write(const char *str, int len) {
    for (int i = 0; i < len; i++) {
        *(char*)(0x10000000UL) = str[i];
    }
}

#include <string.h>  // for strlen() and strcat()
#include <stdlib.h>  // for itoa()
#include <stdarg.h>  // for va_start(), va_end() and va_arg()

void format_to_str(char* out, const char* fmt, va_list args) {
    for(out[0] = 0; *fmt != '\0'; fmt++) {
        if (*fmt != '%') {
            strncat(out, fmt, 1);
        } else {
            fmt++;
            if (*fmt == 's') {
                strcat(out, va_arg(args, char*));
            } else if (*fmt == 'd') {
                itoa(va_arg(args, int), out + strlen(out), 10);
            }
        }
    }
}

int printf(const char* format, ...) {
    char buf[512];
    va_list args;
    va_start(args, format);
    format_to_str(buf, format, args);
    va_end(args);
    terminal_write(buf, strlen(buf));

    return 0;
}

extern char __heap_start, __heap_max;
static char* brk = &__heap_start;
char* _sbrk(int size) {
    if (brk + size > (char*)&__heap_max) {
        terminal_write("_sbrk: heap grows too large\r\n", 29);
        return NULL;
    }

    char* old_brk = brk;
    brk += size;
    return old_brk;
}

#include "queue.h"
#include "thread.h"

/* Student's code goes here (Cooperative Threads). */
/* Define helper functions, if needed, for multi-threading */

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
/* Define helper functions, if needed, for conditional variables */

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

void produce(void* item) {
    while (1) {
        while (count == BUF_SIZE) cv_wait(&nonfull);
        /* At this point, the buffer is not full. */

        /* Student's code goes here (Cooperative Threads). */
        /* Print out producer thread ID and the item pointer */

        /* Student's code ends here. */
        buffer[tail] = item;
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
        /* Print out producer thread ID and the item pointer */

        /* Student's code ends here. */
        void* result = buffer[head];
        head = (head + 1) % BUF_SIZE;
        count -= 1;
        cv_signal(&nonfull);
    }
}

int main() {
    thread_init();

    for (int i = 0; i < 500; i++)
        thread_create(consume, NULL, STACK_SIZE / 16);

    for (int i = 0; i < 500; i++)
        thread_create(produce, NULL, STACK_SIZE / 16);

    printf("main thread exits\n\r");
    thread_exit();
}
