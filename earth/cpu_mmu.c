/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: abstractions of the CPU memory management unit (MMU);
 * physical frame allocation + address translation + protection;
 * physical frames are stored on SD card and there is a cache of 20
 * frames in the memory
 */

#include "egos.h"
#include "earth.h"

#include "mmu.h"
#define INUSE(x) (x & F_INUSE)
#define USE(x)   x |= F_INUSE

/* cached physical frames */
int cache_frame_no[CACHED_NFRAMES];
struct frame* cache = (void*)CACHE_START;
static int cache_read(int frame_no);
static int cache_write(int frame_no, struct frame* src);

/* mapping for address translation */
int curr_vm_pid;
struct mapping mappings[MAX_NFRAMES];

int mmu_alloc(int* frame_no, int* addr) {
    for (int i = 0; i < MAX_NFRAMES; i++) {
        if (!INUSE(mappings[i].flag)) {
            *frame_no = i;
            *addr = cache_read(i);
            USE(mappings[i].flag);
            return 0;
        }
    }
    return -1;
}

int mmu_map(int pid, int page_no, int frame_no, int flag) {
    if (!INUSE(mappings[frame_no].flag)) {
        ERROR("Frame %d has not been allocated", frame_no);
        return -1;
    }

    mappings[frame_no].pid = pid;
    mappings[frame_no].page_no = page_no;
    mappings[frame_no].flag = flag;
    return 0;
}

int mmu_switch(int pid) {
    if (curr_vm_pid != -1) {
        FATAL("context switch not implemented");
    }

    char* base = (void*) VADDR_START;
    for (int i = 0; i < MAX_NFRAMES; i++) {
        if (INUSE(mappings[i].flag)
            && mappings[i].pid == pid) {
            int addr = cache_read(i);
            memcpy(base + PAGE_SIZE * mappings[i].page_no, (char*)addr, PAGE_SIZE);
            INFO("Map frame #%d to page #%d of process #%d", i, mappings[i].page_no, pid);
        }
    }
    return 0;
}

int mmu_init() {
    curr_vm_pid = -1;
    memset(cache_frame_no, 0xff, sizeof(cache_frame_no));
    return 0;
}

/* cache read/write functions */

static int cache_read(int frame_no) {
    int free_no = -1;
    for (int i = 0; i < CACHED_NFRAMES; i++) {
        if (cache_frame_no[i] == frame_no)
            return (int)(cache + i);
        if (cache_frame_no[i] == -1 && free_no == -1)
            free_no = i;
    }

    if (free_no != -1) {
        cache_frame_no[free_no] = frame_no;
        int group = PAGE_SIZE / BLOCK_SIZE;
        disk_read(free_no * group, group, (char*)(cache + free_no));

        return (int)(cache + free_no);
    } else {
        FATAL("Cache is full and eviction is not implemented");
    }
}

