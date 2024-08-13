/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: system support to C library function printf()
 */

#include "egos.h"
#include "servers.h"
#include "string.h"
#include <unistd.h>

/* earth->vsprintf is the C library vsprintf, which converts
 * a format and arguments (e.g., ("example int=%d", 100)) into
 * a string. The string is passed to earth->tty_write() for printing.
 */

#define LOG(x, y)  va_list args; \
                   va_start(args, format); \
                   char str_formatted[256], str_to_print[256]; \
                   uint len = earth->format_to_str(str_formatted, format, args); \
                   strcpy(str_to_print, x); \
                   strcat(str_to_print, str_formatted); \
                   strcat(str_to_print, y); \
                   term_write(str_to_print, (sizeof(x) - 1) + len + (sizeof(y) - 1)); \
                   va_end(args);

int my_printf(const char *format, ...) { LOG("", ""); }

int INFO(const char *format, ...) { LOG("[INFO] ", "\r\n") }

int FATAL(const char *format, ...)
{
    LOG("\x1B[1;31m[FATAL] ", "\x1B[1;0m\r\n") /* red color */
    while(1);
}

int SUCCESS(const char *format, ...)
{
    LOG("\x1B[1;32m[SUCCESS] ", "\x1B[1;0m\r\n") /* green color */
}

int CRITICAL(const char *format, ...)
{
    LOG("\x1B[1;33m[CRITICAL] ", "\x1B[1;0m\r\n") /* yellow color */
}
