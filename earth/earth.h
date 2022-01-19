#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <metal/uart.h>

#include "log.h"

int tty_init();
int tty_read(char* buf, int len);
int tty_write(const char *format, ...);
