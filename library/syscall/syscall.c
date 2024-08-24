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
    if (size > SYSCALL_MSG_LEN) FATAL("sys_send: msg size larger than SYSCALL_MSG_LEN");

    sc->type = SYS_SEND;
    sc->msg.receiver = receiver;
    memcpy(sc->msg.content, msg, size);
    asm("ecall");
}

void sys_recv(int from, int* sender, char* buf, uint size) {
    if (size > SYSCALL_MSG_LEN) FATAL("sys_recv: msg size larger than SYSCALL_MSG_LEN");

    sc->msg.sender = from;
    sc->type = SYS_RECV;
    asm("ecall");

    memcpy(buf, sc->msg.content, size);
    if (sender) *sender = sc->msg.sender;
}
