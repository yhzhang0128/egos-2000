#pragma once

extern struct earth *earth;
#define printf    earth->tty_write

#define INFO      earth->log_info
#define FATAL     earth->log_fatal
#define SUCCESS   earth->log_success
#define HIGHLIGHT earth->log_highlight
