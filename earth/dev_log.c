/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: log printing functions with colors
 */

#include <stdio.h>
#include <stdarg.h>

#define COLOR_RESET  "\x1B[1;0m"

int log_info(const char *format, ...)
{
    printf("[INFO] ");
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\r\n");
}

int log_highlight(const char *format, ...)
{
    printf("%s[HIGHLIGHT] ", "\x1B[1;33m");  // yellow

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s\r\n", COLOR_RESET);
}

int log_success(const char *format, ...)
{
    printf("%s[SUCCESS] ", "\x1B[1;32m");  // green

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s\r\n", COLOR_RESET);
}

int log_fatal(const char *format, ...)
{
    printf("%s[FATAL] ", "\x1B[1;31m"); // red
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s\r\n", COLOR_RESET);

    while(1);
}

