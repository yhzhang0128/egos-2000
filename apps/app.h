#pragma once

#include "mem.h"
#include "egos.h"
#include "print.h"
#include "syscall.h"

struct earth *earth = (void*)EARTH_STRUCT;
#define tty_read earth->tty_read
