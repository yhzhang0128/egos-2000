/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: starter code for implementing printf()
 */

void terminal_write(const char *str, int len) {
    for (int i = 0; i < len; i++) {
        *(char*)(0x10000000) = str[i];
    }
}

#include <stdlib.h>  // for itoa() and utoa()
#include <string.h>  // for strlen() and strcat()
#include <stdarg.h>  // for va_start(), va_end(), va_arg() and va_copy()

char* ulltoa(unsigned long long val, char* str, int radix) {
    char fmt[65];
    const char digits[] = "0123456789abcdef";
    char* pc1 = fmt, *pc2 = str;

    if (radix < 2 || radix > 16 || str == NULL)
        return NULL; 

    do {
        *pc1++ = digits[val % (unsigned)radix];
    } while (val /= (unsigned)radix);

    for (; pc1 != fmt; *pc2++ = *--pc1) { }
    *pc2 = '\0';
    
    return str;
}

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
            } else if (*fmt == 'c') {
                const char c[2] = { (char)va_arg(args, int), '\0' };
                strcat(out, c);
            } else if (*fmt == 'x') {
                itoa(va_arg(args, int), out + strlen(out), 16);
            } else if (*fmt == 'u') {
                utoa(va_arg(args, unsigned int), out + strlen(out), 10);
            } else if (*fmt == 'p') {
               strcat(out, "0x");
               utoa((unsigned)va_arg(args, void*), out + strlen(out), 16);
            } else if (*fmt == 'l') {
               fmt += 2;
               ulltoa(va_arg(args, unsigned long long), out + strlen(out), 10); 
            }
        }
    }
}

int printf(const char* format, ...) {
    char buf[512];
    va_list args;
    va_start(args, format);
    format_to_str(buf, format, args);
    va_end(args);
    terminal_write(buf, strlen(buf));

    return 0;
}

/* Uncomment the code block below when implementing dynamic memory allocation.
 */
/*
extern char __heap_start, __heap_end;
static char* brk = &__heap_start;
char* _sbrk(int size) {
    if (brk + size > (char*)&__heap_end) {
        terminal_write("_sbrk: heap grows too large\r\n", 29);
        return NULL;
    }

    char* old_brk = brk;
    brk += size;
    return old_brk;
}
*/

int main() {
    char* msg = "Hello, World!\n\r";
    terminal_write(msg, 15);

    printf("%s-%d is awesome!\n\r", "egos", 2000);
    printf("%c is character $\n\r", '$');
    printf("%c is character 0\n\r", (char)48);
    printf("%x is integer 1234 in hexadecimal\n\r", 1234);
    printf("%u is the maximum of unsigned int\n\r", (unsigned int)0xFFFFFFFF);
    printf("%p is the hexadecimal address of the hello-world string\n\r", msg);
    printf("%llu is the maximum of unsigned long long\n\r", 0xFFFFFFFFFFFFFFFFULL);

    return 0;
}
