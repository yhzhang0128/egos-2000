#pragma once

int INFO(const char *format, ...);
int HIGHLIGHT(const char *format, ...);
int SUCCESS(const char *format, ...);
int ERROR(const char *format, ...);
int FATAL(const char *format, ...);

struct dev_log {
    int (*log_info)(const char *format, ...);
    int (*log_highlight)(const char *format, ...);
    int (*log_success)(const char *format, ...);
    int (*log_error)(const char *format, ...);
    int (*log_fatal)(const char *format, ...);
};
