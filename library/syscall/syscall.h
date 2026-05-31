#pragma once

#include "servers.h"
#include <string.h>

enum syscall_type {
    SYS_RECV = 1,
    SYS_SEND = 2,
};

#define SYSCALL_MSG_LEN 1024
struct syscall {
    enum syscall_type type; /* SYS_SEND or SYS_RECV */
    int sender;             /* sender process ID    */
    int receiver;           /* receiver process ID  */
    char content[SYSCALL_MSG_LEN]; //holds message that is being sent or received 
    enum { PENDING, DONE } status; //PENDING, if the receiving process or sending process hasnt been scheduled yet
};

void sys_send(int receiver, char* msg, uint size);
void sys_recv(int from, int* sender, char* buf, uint size);
