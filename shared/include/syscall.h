#pragma once

#define SYSCALL_ARGS_BASE  0x8000bc00

enum syscall_type {
	SYS_UNUSED,
	SYS_EXIT,
	SYS_RECV,
	SYS_SEND,
	SYS_NCALLS
};

struct sys_exit {
    int status;
};

struct sys_msg {
    int sender;
    int receiver;
    char msg[1000];
};

struct syscall {
	enum syscall_type type;
	union {
		struct sys_exit exit;
        struct sys_msg msg;
	} args;
    int retval;
};

int sys_send(char* msg);
int sys_recv(char* buf);
void sys_exit(int status);
