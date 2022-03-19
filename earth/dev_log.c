/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: log printing functions with colors
 */

#include <stdio.h>
#include <stdarg.h>

#define KNRM  "\x1B[1;0m"
#define KRED  "\x1B[1;31m"
#define KGRN  "\x1B[1;32m"
#define KYEL  "\x1B[1;33m"

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
    printf("%s[HIGHLIGHT] ", KYEL);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s\r\n", KNRM);
}

int log_success(const char *format, ...)
{
    printf("%s[SUCCESS] ", KGRN);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s\r\n", KNRM);
}

int log_fatal(const char *format, ...)
{
    printf("%s[FATAL] ", KRED);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s\r\n", KNRM);

    while(1);
}

