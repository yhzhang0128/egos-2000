/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: the earth kernel system calls
 */

#include "syscall.h"

#define RISCV_CLINT0_MSIP_BASE 0x2000000
static struct syscall *sc = (struct syscall*)SYSCALL_ARGS_BASE;

static void sys_invoke() {
    *((int*)RISCV_CLINT0_MSIP_BASE) = 1;
    while (sc->type != SYS_UNUSED);
}

int sys_exit(int status){
	sc->type = SYS_EXIT;
	sc->args.exit.status = status;
	sys_invoke();
	return sc->retval;
}
