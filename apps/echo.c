/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple echo
 */


#include "app.h"


int main(int argc, char** argv) {
    if (argc > 1) {
        for (int i = 1; i < argc; i++)
            printf("%s ", (char*)argv + i * CMD_ARG_LEN);
    } else {
        char buf[100];
        tty_read(buf, 100);
        printf("%s", buf);        
    }

    printf("\r\n");
    return 0;
}
