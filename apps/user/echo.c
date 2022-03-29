/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple echo
 */

#include "app.h"

#define tty_read earth->tty_read

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++)
        printf("%s ", argv[i]);

    printf("\r\n");
    return 0;
}
