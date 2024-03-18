/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple cat
 */

#include "app.h"
#include <string.h>

i32 main(i32 argc, char** argv) {
    if (argc == 1) {
        INFO("usage: cat [FILE]");
        return -1;
    }

    /* Get the inode number of the file */
    i32 file_ino = dir_lookup(grass->workdir_ino, argv[1]);
    if (file_ino < 0) {
        INFO("cat: file %s not found", argv[1]);
        return -1;
    }

    /* Read and print the first block of the file */
    char buf[BLOCK_SIZE];
    file_read(file_ino, 0, buf);
    printf("%s", buf);
    if (buf[strlen(buf) - 1] != '\n') printf("\r\n");

    return 0;
}
