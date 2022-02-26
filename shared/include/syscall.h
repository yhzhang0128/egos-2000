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

struct syscall {
	enum syscall_type type;
	union {
		struct sys_exit exit;
	} args;
    int retval;
};

int sys_exit(int status);
