/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: system support to C library function printf()
 */

#include "egos.h"
#include <unistd.h>

/* printf() is linked from the compiler's C library;
 * printf() constructs a string based on its arguments
 * and prints the string to the tty device by calling _write().
 */

int _write(int file, char *ptr, int len) {
    if (file != STDOUT_FILENO) return -1;
    return earth->tty_write(ptr, len);
}

int _close(int file) { return -1; }
int _fstat(int file, void *stat) { return -1; }
int _lseek(int file, int ptr, int dir) { return -1; }
int _read(int file, void *ptr, int len) { return -1; }
int _isatty(int file) { return (file == STDOUT_FILENO); }
