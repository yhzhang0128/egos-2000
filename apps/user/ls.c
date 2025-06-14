/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple ls
 */

#include "app.h"
#include <string.h>

int main(int argc, char** argv) {
    if (argc > 1) {
        INFO("ls: ls with args is not implemented");
        return -1;
    }

    /* Read the directory content. */
    char buf[BLOCK_SIZE];
    file_read(workdir_ino, 0, buf);

    /* Remove the inode numbers from the string. */
    for (uint i = 1; i < strlen(buf); i++)
        if (buf[i - 1] == ' ' && buf[i] >= '0' && buf[i] <= '9') buf[i] = ' ';

    /* Print out the directory content. */
    printf("%s\n\r", buf);
    return 0;
}
