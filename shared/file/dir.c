#include <string.h>

#include "egos.h"
#include "print.h"
#include "syscall.h"
#include "servers.h"

int dir_lookup(int dir_ino, char* name) {
    int sender;
    struct dir_request req;
    char buf[SYSCALL_MSG_LEN];

    req.type = DIR_LOOKUP;
    req.ino = dir_ino;
    strcpy(req.name, name);
    sys_send(GPID_DIR, (void*)&req, sizeof(struct dir_request));
    sys_recv(&sender, buf, SYSCALL_MSG_LEN);
    if (sender != GPID_DIR)
        FATAL("sys_shell expects message from GPID_DIR");

    struct dir_reply *reply = (void*)buf;
    return reply->ino;
}
