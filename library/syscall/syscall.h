#pragma once

#include "servers.h"

enum syscall_type {
	SYS_UNUSED,
	SYS_RECV,
	SYS_SEND,
	SYS_NSYSCALL /* egos-2000 has only 2 system calls */
};

struct sys_msg {
    int sender;
    int receiver;
    char content[SYSCALL_MSG_LEN];
    enum {PENDING, RECEIVED} status;
};

struct syscall {
    enum syscall_type type;  /* Type of the system call */
    struct sys_msg msg;      /* Data of the system call */
};

void sys_send(int receiver, char* msg, uint size);
void sys_recv(int from, int* sender, char* buf, uint size);
