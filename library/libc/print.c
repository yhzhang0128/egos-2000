/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: formatted printing (e.g., printf)
 * format_to_str() converts a format to a C string,
 * e.g., converts ("%s-%d", "egos", 2000) to "egos-2000"
 * term_write() prints a C string to the screen
 */

#include "egos.h"
#include "servers.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

void format_to_str(char* out, const char* fmt, va_list args) {
    for(out[0] = 0; *fmt != '\0'; fmt++) {
        if (*fmt != '%') {
            strncat(out, fmt, 1);
        } else {
            fmt++;
            if (*fmt == 's') {
                strcat(out, va_arg(args, char*));
            } else if (*fmt == 'd') {
                itoa(va_arg(args, int), out + strlen(out), 10);
            }
            /* Student's code goes here (Hello, World!) */

            /* Handle format %c, %x, %u, etc. */
            /* If not matching any pattern, simply print the '%' symbol */

            /* Student's code ends here. */
        }
    }
}

#define LOG(prefix, suffix) char buf[512]; \
                            strcpy(buf, prefix); \
                            va_list args; \
                            va_start(args, format); \
                            format_to_str(buf + strlen(prefix), format, args); \
                            va_end(args); \
                            strcat(buf, suffix); \
                            term_write(buf, strlen(buf));

int my_printf(const char* format, ...) {
    LOG("", "");
}

int INFO(const char* format, ...) {
    LOG("[INFO] ", "\r\n")
}

int FATAL(const char* format, ...) {
    LOG("\x1B[1;31m[FATAL] ", "\x1B[1;0m\r\n") /* red color */
    while(1);
}

int SUCCESS(const char* format, ...) {
    LOG("\x1B[1;32m[SUCCESS] ", "\x1B[1;0m\r\n") /* green color */
}

int CRITICAL(const char* format, ...) {
    LOG("\x1B[1;33m[CRITICAL] ", "\x1B[1;0m\r\n") /* yellow color */
}
