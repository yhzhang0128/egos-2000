/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple ls
 */

#include "app.h"
#include <string.h>

int main(int argc, char** argv) {
    if (argc > 1) {
        INFO("ls with args not implemented");
        return -1;
    }

    char result[BLOCK_SIZE];
    file_read(grass->work_dir_ino, 0, result);
    
    for (int i = 1; i < strlen(result); i++)
        if (result[i - 1] == ' ' &&
            result[i] >= '0' && result[i] <= '9')
            result[i] = ' ';

    printf("%s\r\n", result);

    return 0;
}
