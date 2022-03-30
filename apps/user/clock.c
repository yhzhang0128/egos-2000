/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple clock
 */

#include "app.h"
#include <stdlib.h>

int main(int argc, char** argv) {
    volatile int i, j, cnt = (argc == 1)? 100 : atoi(argv[1]);
    
    for (i = 0; i < cnt; i++) {
        for (j = 0; j < 5000000; j++);
        printf("clock: tick#%d / #%d\r\n", i + 1, cnt);
    }

    return 0;
}
