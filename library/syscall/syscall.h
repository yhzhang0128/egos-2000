#pragma once

#define SYSCALL_MSG_LEN        1024
#define RISCV_CLINT0_MSIP_BASE 0x2000000

enum syscall_type {
	SYS_UNUSED,
	SYS_RECV,
	SYS_SEND,
	SYS_NCALLS
};

struct sys_msg {
    int sender;
    int receiver;
    char msg[SYSCALL_MSG_LEN];
};

struct syscall {
	enum syscall_type type;
	union {
            struct sys_msg msg;
	} payload;
    int retval;
};

void sys_exit(int status);
int sys_send(int pid, char* msg, int size);
int sys_recv(int* pid, char* buf, int size);

#include "servers.h"
