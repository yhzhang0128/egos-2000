#pragma once

#include <stdlib.h>

#include "elf.h"
#include "mmu.h"
#include "print.h"
#include "syscall.h"

#define RISCV_CLINT0_MSIP_BASE           0x2000000
#define RISCV_CLINT0_MTIME_BASE          0x200bff8
#define RISCV_CLINT0_MTIMECMP_BASE       0x2004000

void timer_init();
long long timer_reset();
