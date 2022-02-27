/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: inode layer of the file system
 */

#include "app.h"

int main() {
    SUCCESS("Enter kernel process GPID_FILE");

    static int cnt = 0;
    while (1) {
        if (cnt++ % 50000 == 0)
            HIGHLIGHT("In sys_file");
    }
    return 0;
}
