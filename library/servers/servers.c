/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: user friendly interfaces of system processes
 */

#include "egos.h"
#include "servers.h"
#include <stdlib.h>
#include <string.h>

static int sender;
static char buf[SYSCALL_MSG_LEN];

void exit(int status) {
    grass->sys_exit(status);
    while(1);
}

/* To understand directory management, read tools/mkfs.c */
int dir_lookup(int dir_ino, char* name) {
    char buf[BLOCK_SIZE];
    file_read(dir_ino, 0, buf);

    for (uint i = 0, namelen = strlen(name); i < strlen(buf) - namelen; i++)
        if (!strncmp(name, buf + i, namelen) &&
            buf[i + namelen] == ' ' && (i == 0 || buf[i - 1] == ' '))
            return atoi(buf + i + namelen);

    return -1;
}

int file_read(int file_ino, uint offset, char* block) {
    struct file_request req;
    req.type = FILE_READ;
    req.ino = file_ino;
    req.offset = offset;

    grass->sys_send(GPID_FILE, (void*)&req, sizeof(req));
    grass->sys_recv(GPID_FILE, &sender, buf, SYSCALL_MSG_LEN);
    
    struct file_reply *reply = (void*)buf;
    memcpy(block, reply->block.bytes, BLOCK_SIZE);

    return reply->status == FILE_OK? 0 : -1;
}
