/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: system support to C library function printf()
 */

#include "egos.h"
#include <unistd.h>

/* earth->vsprintf is the C library vsprintf, which converts
 * a format and arguments (e.g., arguments to printf) into a
 * string. This string is then passed to earth->tty_write().
 */

#define LOG(x, y)  earth->tty_write(x, sizeof(x)); \
                   va_list args; \
                   va_start(args, format); \
                   char str_to_print[256]; \
                   uint len = earth->tty_vsprintf(str_to_print, format, args); \
                   earth->tty_write(str_to_print, len); \
                   va_end(args); \
                   earth->tty_write(y, sizeof(y));

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
