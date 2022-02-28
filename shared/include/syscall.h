#pragma once

#define SYSCALL_MSG_LEN        1024
#define RISCV_CLINT0_MSIP_BASE 0x2000000

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
    char msg[SYSCALL_MSG_LEN];
};

struct syscall {
	enum syscall_type type;
	union {
            struct sys_exit exit;
            struct sys_msg msg;
	} payload;
    int retval;
};

void sys_exit(int status);
int sys_recv(char* buf, int size);
int sys_send(int pid, char* msg, int size);
