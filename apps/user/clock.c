/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: a simple clock
 */

#include "app.h"
#include <string.h>

int main(int argc, char** argv) {
    int silent = (argc == 2)? (strcmp(argv[1], "--silent") == 0) : 0;

    for (uint i = 0; 1; i++) {
        for (uint j = 0; j < 5000000; j++);
        if (!silent) printf("clock: tick#%d\r\n", i);
    }

    return 0;
}
