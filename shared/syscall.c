/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: the earth kernel system calls
 */

#include <string.h>
#include "syscall.h"

#define RISCV_CLINT0_MSIP_BASE 0x2000000
static struct syscall *sc = (struct syscall*)SYSCALL_ARGS_BASE;

static void sys_invoke() {
    *((int*)RISCV_CLINT0_MSIP_BASE) = 1;
    while (sc->type != SYS_UNUSED);
}

int sys_send(int receiver, char* msg, int size) {
    if (size > SYSCALL_MSG_LEN)
        return -1;

    sc->type = SYS_SEND;
    sc->payload.msg.receiver = receiver;
    memcpy(sc->payload.msg.msg, msg, size);
    sys_invoke();
    return sc->retval;    
}

int sys_recv(char* buf, int size) {
    if (size > SYSCALL_MSG_LEN)
        return -1;

    sc->type = SYS_RECV;
    sys_invoke();
    memcpy(buf, sc->payload.msg.msg, size);
    return sc->retval;
}

void sys_exit(int status) {
	sc->type = SYS_EXIT;
	sc->payload.exit.status = status;
	sys_invoke();
}
