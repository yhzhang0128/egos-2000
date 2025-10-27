/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple cat
 */

#include "app.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        INFO("usage: cat [FILE]");
        return -1;
    }

    /* Get the inode number of the file. */
    int file_ino = dir_lookup(workdir_ino, argv[1]);
    if (file_ino < 0) {
        INFO("cat: file %s not found", argv[1]);
        return -1;
    }

    /* Read and print the first block of the inode. */
    char buf[BLOCK_SIZE];
    file_read(file_ino, 0, buf);
    printf("%s", buf);
    if (buf[strlen(buf) - 1] != '\n') printf("\n\r");

    return 0;
}
