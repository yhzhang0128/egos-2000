#include <stddef.h>

/* heap start/end are defined in the memory layout scripts */
extern char HEAP_START;
extern char HEAP_END;

static char* brk = &HEAP_START;

char* _sbrk(ptrdiff_t incr) {
    char* old = brk;

    if (brk + incr >= &HEAP_END)
        return (void*) -1;

    brk += incr;
    return old;
}
