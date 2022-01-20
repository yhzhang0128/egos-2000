/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: echo
 */


#include "app.h"

int main() {
    char buf[100];
    
    while (1) {
        read(buf, 100);
        SUCCESS("echo got: %s", buf);
    }
}
