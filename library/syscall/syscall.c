/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: the system call interface for applications
 */

#include "egos.h"
#include "syscall.h"

static struct syscall* sc = (struct syscall*)SYSCALL_ARG;

void sys_send(int receiver, char* msg, uint size) {
    sc->type     = SYS_SEND;
    sc->receiver = receiver;
    memcpy(sc->content, msg, size);
    asm("ecall"); //triggers an environment call exception, which will be handled by excp_entry in kernel.c
}

void sys_recv(int from, int* sender, char* buf, uint size) {
    sc->type   = SYS_RECV;
    sc->sender = from;
    asm("ecall"); //triggers an environment call exception, which will be handled by excp_entry in kernel.c
    memcpy(buf, sc->content, size);
    if (sender) *sender = sc->sender;
}
