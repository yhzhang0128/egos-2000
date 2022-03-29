#pragma once

extern struct earth *earth;
#define printf    earth->tty_write
#define INFO      earth->tty_info
#define FATAL     earth->tty_fatal
#define SUCCESS   earth->tty_success
#define HIGHLIGHT earth->tty_highlight
