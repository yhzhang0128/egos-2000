#include "syscall.h"
#include "process.h"

#define EXCP_ID_ECALL_U    8
#define EXCP_ID_ECALL_M    11

#define INTR_ID_SOFT       3
#define INTR_ID_TIMER      7

struct pending_ipc
{
    int in_use;
    int sender;
    int receiver;
    char msg[SYSCALL_MSG_LEN];
};

extern struct pending_ipc *msg_buffer;

void intr_entry(uint);
void excp_entry(uint);
void kernel_entry(uint, uint);
