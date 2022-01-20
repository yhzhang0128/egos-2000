/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize the process control block and 
 * spawns some kernel processes, including file system and shell
 */

#include "egos.h"
#include "grass.h"

struct earth *earth = (void*)EARTH_ADDR;

int main() {
    SUCCESS("Enter the grass layer");
    char* buf = malloc(512);
    free(buf);
    INFO("stack variable @0x%.8x and heap variable @0x%.8x", &buf, buf);
    return 0;
}
