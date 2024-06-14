/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: a simple echo
 */

#include "app.h"

int main(int argc, char** argv) {
    for (uint i = 1; i < argc; i++) printf("%s ", argv[i]);
    printf("\r\n");
    return 0;
}
