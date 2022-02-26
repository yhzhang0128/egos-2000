#pragma once

#include "egos.h"
#include "print.h"
#include "syscall.h"

struct earth *earth = (void*)EARTH_ADDR;
#define read      earth->tty_read
