#pragma once

#include "servers.h"

enum syscall_type {
	SYS_UNUSED,
	SYS_RECV,
	SYS_SEND,
	SYS_NCALLS
};

struct sys_msg {
    int sender;
    int receiver;
    char content[SYSCALL_MSG_LEN];
};

struct syscall {
    enum syscall_type type;
    struct sys_msg msg;
    int retval;
};

void sys_exit(int status);
int sys_send(int pid, char* msg, int size);
int sys_recv(int* pid, char* buf, int size);

