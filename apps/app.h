#pragma once

#include "egos.h"
#include "print.h"
#include "syscall.h"

struct earth *earth = (void*)KERNEL_STACK_TOP;

#define tty_read earth->tty_read
