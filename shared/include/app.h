#pragma once

#include "egos.h"

struct earth *earth = (void*)EARTH_ADDR;
#define printf    earth->tty_write

#define INFO      earth->log.log_info
#define HIGHLIGHT earth->log.log_highlight
#define SUCCESS   earth->log.log_success
#define ERROR     earth->log.log_error
#define FATAL     earth->log.log_fatal
