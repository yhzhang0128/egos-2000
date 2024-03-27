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
    int cnt = (argc == 1)? 1000 : atoi(argv[1]);

    for (uint i = 0; i < cnt; i++) {
        for (uint j = 0; j < 5000000; j++);
        printf("clock: tick#%d / #%d\r\n", i + 1, cnt);
    }

    return 0;
}
