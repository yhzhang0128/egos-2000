/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: a program corrupting the memory of other processes
 * Students are asked to modify the grass kernel so that this 
 * program terminates gracefully without harming other processes.
 */

#include "app.h"
#include "egos.h"
#include <string.h>

int main() {
    memset((void*)APPS_PAGES_BASE, 0, RAM_END - APPS_PAGES_BASE);
    /* If the OS protects memory correctly,
     * this memset should trigger an exception, killing this application;
     * Otherwise, the following message will be printed
     */
    SUCCESS("Crash2 succeeds in corrupting the memory of other processes");
}
