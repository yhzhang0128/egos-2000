/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple ls
 */

#include "app.h"
#include <string.h>

i32 main(i32 argc, char** argv) {
    if (argc > 1) {
        INFO("ls: ls with args not implemented");
        return -1;
    }

    /* Read the directory content */
    char buf[BLOCK_SIZE];
    file_read(grass->workdir_ino, 0, buf);
    
    /* Remove the inode numbers from the string */
    for (u32 i = 1; i < strlen(buf); i++)
        if (buf[i - 1] == ' ' && buf[i] >= '0' && buf[i] <= '9') buf[i] = ' ';

    /* Print the directory content */
    printf("%s\r\n", buf);
    return 0;
}
