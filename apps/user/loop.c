/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple loop
 */

#include "app.h"
#include <stdlib.h>

int main(int argc, char** argv) {
    for (uint i = 0; argc == 1 ? 1 : (i < atoi(argv[1])); i++) {
        for (uint j = 0; j < 1000000; j++);
        if (argc != 1 && argc != 3) printf("loop #%d\n\r", i);
    }
    return 0;
}
