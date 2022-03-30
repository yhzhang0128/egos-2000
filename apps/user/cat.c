/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple cat
 */

#include "app.h"
#include <string.h>

int main(int argc, char** argv) {
    if (argc == 1) {
        INFO("cat: please provide an argument");
        return 0;
    }

    /* Get the inode number of the file */
    int file_ino = dir_lookup(grass->work_dir_ino, argv[1]);
    if (file_ino < 0) {
        INFO("cat: file %s not found", argv[1]);
        return -1;
    }

    /* Read and print the first block of the file */
    char result[BLOCK_SIZE];
    file_read(file_ino, 0, result);

    printf("%s", result);
    if (result[strlen(result) - 1] != '\n') printf("\r\n");

    return 0;
}
