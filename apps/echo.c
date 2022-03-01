/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: echo
 */


#include "app.h"


int main(int argc, char** argv) {
    char buf[100];

    INFO("echo got work dir: %s", grass->work_dir);
    INFO("echo got work dir name: %s", grass->work_dir_name);
    INFO("echo got %d args", argc);
    for (int i = 0; i < argc; i++)
        printf("%s\r\n", (char*)argv + i * CMD_ARG_LEN);


    int cnt = 0;
    while (1) {
        tty_read(buf, 100);
        SUCCESS("echo got: %s", buf);
        
        sys_exit(cnt++);
    }
}
