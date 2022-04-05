#include "bus_uart.c"

typedef void (*func_t)();

extern func_t metal_constructors_start;
extern func_t metal_constructors_end;
extern func_t metal_destructors_start;
extern func_t metal_destructors_end;

func_t metal_tty_putc = (void*)uart_putc;

void metal_init_run(void) {
    /* Make sure the constructors only run once */
    static int init_done = 0;
    if (init_done) {
        return;
    }
    init_done = 1;

    if (&metal_constructors_end <= &metal_constructors_start) {
        return;
    }

    func_t *funcptr = &metal_constructors_start;
    while (funcptr != &metal_constructors_end) {
        func_t func = *funcptr;

        func();

        funcptr += 1;
    }
}


void metal_fini_run(void) {
    /* Make sure the destructors only run once */
    static int fini_done = 0;
    if (fini_done) {
        return;
    }
    fini_done = 1;

    if (&metal_destructors_end <= &metal_destructors_start) {
        return;
    }

    func_t *funcptr = &metal_destructors_start;
    while (funcptr != &metal_destructors_end) {
        func_t func = *funcptr;

        func();

        funcptr += 1;
    }
}

__attribute__((section(".init"))) void __metal_synchronize_harts() {}
