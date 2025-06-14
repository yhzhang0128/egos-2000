/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple echo
 */

#include "app.h"

int main(int argc, char** argv) {
    for (uint i = 1; i < argc; i++) printf("%s ", argv[i]);
    printf("\n\r");
    return 0;
}
