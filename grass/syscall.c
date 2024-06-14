/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: the system call interface
 */

#include "egos.h"
#include "syscall.h"
#include <string.h>

static struct syscall *sc = (struct syscall*)SYSCALL_ARG;

static void sys_invoke() {
    /* The standard way of system call is using the `ecall` instruction; 
     * Switching to ecall is given to students as an exercise */
    *((int*)MSIP) = 1;
    while (sc->type != SYS_UNUSED);
}

void sys_send(int receiver, char* msg, uint size) {
    if (size > SYSCALL_MSG_LEN) FATAL("sys_send: msg size larger than SYSCALL_MSG_LEN");

    sc->type = SYS_SEND;
    sc->msg.receiver = receiver;
    memcpy(sc->msg.content, msg, size);
    sys_invoke(); 
}

void sys_recv(int from, int* sender, char* buf, uint size) {
    if (size > SYSCALL_MSG_LEN) FATAL("sys_recv: msg size larger than SYSCALL_MSG_LEN");

    sc->msg.sender = from;
    sc->type = SYS_RECV;
    sys_invoke();
    memcpy(buf, sc->msg.content, size);
    if (sender) *sender = sc->msg.sender;
}

void sys_exit(int status) {
    struct proc_request req;
    req.type = PROC_EXIT;
    sys_send(GPID_PROCESS, (void*)&req, sizeof(req));
}
