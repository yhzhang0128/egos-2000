/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: formatted printing
 * format_to_str() converts a format into a C string:
 * e.g., converts ("%s-%d", "egos", 2000) to "egos-2000".
 * term_write() prints the converted C string to the screen.
 */

#include "egos.h"
#include "servers.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

void format_to_str(char* out, const char* fmt, va_list args) {
    for (out[0] = 0; *fmt != '\0'; fmt++) {
        if (*fmt != '%') {
            strncat(out, fmt, 1);
        } else {
            fmt++;
            switch (*fmt) {
                case 's':
                    strcat(out, va_arg(args, char*)); //va_arg(args, Type of next parameter)
                    break;
                case 'd':
                    // integer to string; integer, memory location; initial pointer of out, then string length, base
                    itoa(va_arg(args, int), out + strlen(out), 10);
                    break;
                case 'c':
                    char c = va_arg(args,int);
                    char s[2];
                    s[0] = c;
                    s[1] = '\0';
                    strcat(out, s);
                    break;
                case 'x':
                    itoa(va_arg(args,int), out + strlen(out), 16);
                    break;
                
                case 'p':
                    char* msg = va_arg(args, char*); // pointer, need to cast to int for itoa
                    strcat(out, "0x");
                    itoa((int)msg, out + strlen(out), 16);
                    break;
                
                case 'u':{
                    const size_t INT_LEN = 21;
                    char uint_arr[INT_LEN];
                    uint_arr[INT_LEN-1] = '\0';
                    int ind = INT_LEN-2;

                    unsigned int res = va_arg(args, unsigned int); // write digit by digit         
                    if(res == 0){
                        uint_arr[ind--] = '0';
                    }              
                    while(res > 0){ 
                        uint_arr[ind] = (res%10) + '0';
                        ind--;
                        res/=10;
                    }
                    strcat(out, &uint_arr[ind+1]);
                    break;
                }
                case 'l':{
                    const size_t INT_LEN = 21;
                    char uint_arr[INT_LEN];
                    uint_arr[INT_LEN-1] = '\0';
                    int ind = INT_LEN-2;
                    const char* lu = fmt;
                    lu++;
                    if(*lu == 'l' && *(lu+1) == 'u'){
                        unsigned long long res = va_arg(args, unsigned long long);
                        if(res == 0){
                            uint_arr[ind--] = '0';
                        }       
                        while(res > 0){ 
                            uint_arr[ind] = (res%10) + '0';
                            ind--;
                            res/=10;
                        }
                        strcat(out, &uint_arr[ind+1]);
                        fmt += 2;
                        break;
                    }
                    break;
                }
            }
            /* Student's code goes here (Hello, World!). */

            /* Handle format %c, %u, %p, %lld, %llu and %llx. */

            /* Student's code ends here. */
        }
    }
}

#define LOG(prefix, suffix)                                                    \
    char buf[512];                                                             \
    strcpy(buf, prefix);                                                       \
    va_list args;                                                              \
    va_start(args, format);                                                    \
    format_to_str(buf + strlen(prefix), format, args);                         \
    va_end(args);                                                              \
    strcat(buf, suffix);                                                       \
    term_write(buf, strlen(buf));

int my_printf(const char* format, ...) { LOG("", ""); }

int INFO(const char* format, ...) { LOG("[INFO] ", "\n\r") }

int FATAL(const char* format, ...) {
    LOG("\x1B[1;31m[FATAL] ", "\x1B[1;0m\n\r") /* \x1B[1;31m means red. */
    while (1);
}

int SUCCESS(const char* format, ...) {
    LOG("\x1B[1;32m[SUCCESS] ", "\x1B[1;0m\n\r") /* \x1B[1;32m means green. */
}

int CRITICAL(const char* format, ...) {
    LOG("\x1B[1;33m[CRITICAL] ", "\x1B[1;0m\n\r") /* \x1B[1;33m means yellow. */
}
