/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple clock
 */

#include "app.h"

int main(int argc, char** argv) {
    volatile int i, cnt = 0;
    while (1) {
        for (i = 0; i < 5000000; i++);
        printf("\r\nclock: tick#%d\r\n", argv[1], ++cnt);
    }
}
