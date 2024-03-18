/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple echo
 */

#include "app.h"

i32 main(i32 argc, char** argv) {
    for (u32 i = 1; i < argc; i++) printf("%s ", argv[i]);
    printf("\r\n");
    return 0;
}
