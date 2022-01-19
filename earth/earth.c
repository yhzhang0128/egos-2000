/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize dev_tty, dev_disk, cpu_intr and cpu_mmu
 */


#include "egos.h"
#include "earth.h"

static struct earth earth;

void test_tty();

int main() {
    if (tty_init()) {
        ERROR("Failed at initializing tty device");
        return -1;
    }
    earth.tty_read = tty_read;
    earth.tty_write = tty_write;

    test_tty();

    return 0;
}    

void test_tty() {
    char buf[100];
    while (1) {
        INFO("This is the earthbox. Enter a sentence:");

        tty_read(buf, 100);

        INFO("Got sentence: %s", buf);
    }
}
