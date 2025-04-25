/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a program trying to corrupt memory
 * Students are asked to modify the kernel so that this
 * program terminates gracefully without corrupting any memory.
 */

#include "app.h"
#include "egos.h"
#include <string.h>

int main() {
    memset((void*)APPS_PAGES_BASE, 0, RAM_END - APPS_PAGES_BASE);
    return 0;
}
