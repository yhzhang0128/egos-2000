#include <stdio.h>
#include <stdarg.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"

int ERROR(const char *format, ...)
{
    printf("%s[ERROR] ", KRED);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s\r\n", KNRM);
}

int SUCCESS(const char *format, ...)
{
    printf("%s[SUCCESS] ", KGRN);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s\r\n", KNRM);
}
