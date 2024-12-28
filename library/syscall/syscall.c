/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: the system call interface for applications
 */

#include "egos.h"
#include "syscall.h"
#include <string.h>

static struct syscall* sc = (struct syscall*)SYSCALL_ARG;

void sys_send(int receiver, char* msg, uint size) {
    sc->type     = SYS_SEND;
    sc->receiver = receiver;
    memcpy(sc->content, msg, size);
    asm("ecall");
}

void sys_recv(int from, int* sender, char* buf, uint size) {
    sc->sender = from;
    sc->type   = SYS_RECV;
    asm("ecall");
    memcpy(buf, sc->content, size);
    if (sender) *sender = sc->sender;
}
