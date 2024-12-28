#pragma once

#include "servers.h"

enum syscall_type {
    SYS_UNUSED,
    SYS_RECV, /* 1 */
    SYS_SEND, /* 2 */
};

struct syscall {
    enum syscall_type type; /* SYS_SEND or SYS_RECV */
    int sender;             /* sender process ID    */
    int receiver;           /* receiver process ID  */
    char content[SYSCALL_MSG_LEN];
    enum { PENDING, DONE } status;
};

void sys_send(int receiver, char* msg, uint size);
void sys_recv(int from, int* sender, char* buf, uint size);
