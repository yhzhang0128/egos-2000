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
    enum syscall_type type;  /* Type of the system call */
    struct sys_msg msg;      /* Data of the system call */
};

struct pending_ipc
{
    int in_use;
    int sender;
    int receiver;
    char msg[SYSCALL_MSG_LEN];
};

extern struct pending_ipc *msg_buffer;

void sys_exit(int status);
void sys_send(int receiver, char* msg, uint size);
void sys_recv(int from, int* sender, char* buf, uint size);
