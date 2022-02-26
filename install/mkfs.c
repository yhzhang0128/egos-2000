/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: create the disk image file (disk.img)
 * the first 1MB is reserved as 256 physical frames for paging
 * the second 1MB contains some ELF binary executables for booting
 * the last 4MB is managed by a file system
 * in total, disk.img should be 6MB
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>

char* kernel_processes[] = {
                            "bin/release/grass.elf",
                            "bin/release/sys_proc.elf",
                            //"bin/release/sys_dir.elf",
                            //"bin/release/sys_shell.elf"
};
char buf[1024 * 1024];


int main() {
    freopen("disk.img", "w", stdout);

    /* paging area */
    memset(buf, 0, sizeof(buf));
    write(1, buf, 1024 * 1024);

    /* grass kernel processes */
    int n = sizeof(kernel_processes) / sizeof(char*);
    if (n > 8) {
        fprintf(stderr, "[ERROR] more than 8 kernel processes\n");
        return -1;
    }
    
    fprintf(stderr, "[INFO] Loading %d kernel processes\n", n);
    for (int i = 0; i < n; i++) {
        struct stat st;
        stat(kernel_processes[i], &st);
        fprintf(stderr, "[INFO] Loading %s: %ld bytes\n", kernel_processes[i], (long)st.st_size);
        assert(st.st_size > 0);

        if (st.st_size > 128 * 1024) {
            fprintf(stderr, "[ERROR] file larger than 128KB\n");
            return -1;
        }

        freopen(kernel_processes[i], "r", stdin);
        int nread = 0;
        while (nread < st.st_size)
            nread += read(0, buf + nread, 128 * 1024 - nread);

        write(1, buf, st.st_size);
        write(1, buf, 128 * 1024 - st.st_size);
    }

    for (int i = 0; i < 8 - n; i++)
        write(1, buf, 128 * 1024);
        
    /* file system */
    
    fclose(stdout);
    return 0;
}
