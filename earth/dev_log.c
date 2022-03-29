/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: log printing functions with colors
 */

#include <stdio.h>
#include <stdarg.h>

#define VPRINTF   va_list args; \
                  va_start(args, format); \
                  vprintf(format, args); \
                  va_end(args); \

int log_info(const char *format, ...)
{
    printf("[INFO] ");
    VPRINTF
    printf("\r\n");
}

int log_highlight(const char *format, ...)
{
    printf("%s[HIGHLIGHT] ", "\x1B[1;33m"); // yellow
    VPRINTF
    printf("%s\r\n", "\x1B[1;0m");
}

int log_success(const char *format, ...)
{
    printf("%s[SUCCESS] ", "\x1B[1;32m");   // green
    VPRINTF
    printf("%s\r\n", "\x1B[1;0m");
}

int log_fatal(const char *format, ...)
{
    printf("%s[FATAL] ", "\x1B[1;31m");     // red
    VPRINTF
    printf("%s\r\n", "\x1B[1;0m");
    while(1);
}
