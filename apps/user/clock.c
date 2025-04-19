/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple clock
 */

#include "app.h"
#include <string.h>

int main(int argc, char** argv) {
    int silent = 0;
    if ((argc == 2) &&
        (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--silent") == 0)) {
        silent = 1;
    }

    for (uint i = 0; i<60; i++) {
        for (uint j = 0; j < 5000000; j++);
        if (!silent) printf("clock: tick#%d\r\n", i);
    }

    return 0;
}
