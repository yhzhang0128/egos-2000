/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a program triggering a memory exception
 * Students are asked to modify the kernel so that this
 * program terminates gracefully without crashing the kernel.
 */

#include "app.h"
#include <stdlib.h>

int main() {
    char* heap_overflow = malloc(32 * 1024 * 1024);
    return 0;
}
