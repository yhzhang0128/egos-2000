/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a program corrupting the memory of other processes
 * Students are asked to modify the grass kernel so that this 
 * program crashes and terminates without harming other processes.
 */

#include "app.h"
#include <string.h>

int main() {
    memset((void*)FRAME_CACHE_START, 0, FRAME_CACHE_END - FRAME_CACHE_START);
    /* If the OS protects memory correctly,
     * this memset should trigger an exception, killing this application;
     * Otherwise, the following message will be printed
     */
    SUCCESS("Crash2 succeeds in corrupting the memory of other processes");
}
