/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple clock
 */

#include "app.h"
#include <stdlib.h>

i32 main(i32 argc, char** argv) {
    i32 cnt = (argc == 1)? 1000 : atoi(argv[1]);

    for (u32 i = 0; i < cnt; i++) {
        for (u32 j = 0; j < 5000000; j++);
        printf("clock: tick#%d / #%d\r\n", i + 1, cnt);
    }

    return 0;
}
