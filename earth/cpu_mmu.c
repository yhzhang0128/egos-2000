/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: abstractions of the memory management unit (MMU);
 * there are 256 physical frames at the start of the SD card and 28 of
 * them are cached in the memory (more precisely, in the DTIM cache)
 */

#include "egos.h"
#include "earth.h"
#include <stdlib.h>

struct frame_t {
    char byte[PAGE_SIZE];
};

struct translation_table_t {
    struct {
        int pid;
        int page_no;
        int flag;
    } frame[NFRAMES];
} trans_table;

#define INUSE(x) (x.flag & F_INUSE)
#define USE(x)   x.flag |= F_INUSE

int curr_vm_pid;
int cache_frame_no[CACHED_NFRAMES];
struct frame_t* cache = (void*)FRAME_CACHE_START;

static int cache_read(int frame_no);
static int cache_write(int frame_no, struct frame_t* src);

int mmu_init() {
    curr_vm_pid = -1;
    memset(cache_frame_no, 0xff, sizeof(cache_frame_no));
    memset(&trans_table, 0, sizeof(struct translation_table_t));

    return 0;
}

int mmu_alloc(int* frame_no, int* cached_addr) {
    for (int i = 0; i < NFRAMES; i++) {
        if (!INUSE(trans_table.frame[i])) {
            *frame_no = i;
            *cached_addr = cache_read(i);
            USE(trans_table.frame[i]);
            return 0;
        }
    }
    return -1;
}

int mmu_free(int pid) {
    for (int i = 0; i < NFRAMES; i++) {
        if (trans_table.frame[i].pid == pid &&
            INUSE(trans_table.frame[i])) {
            /* remove the mapping */
            trans_table.frame[i].pid = 0;
            trans_table.frame[i].page_no = 0;
            trans_table.frame[i].flag = 0;

            /* invalidate the cache */
            for (int j = 0; j < CACHED_NFRAMES; j++)
                if (cache_frame_no[j] == i)
                    cache_frame_no[j] = -1;
        }
    }
    return 0;
}

int mmu_map(int pid, int page_no, int frame_no, int flag) {
    if (flag != F_ALL)
        FATAL("Memory protection not implemented");
    
    if (!INUSE(trans_table.frame[frame_no])) {
        INFO("Frame %d has not been allocated", frame_no);
        return -1;
    }

    trans_table.frame[frame_no].pid = pid;
    trans_table.frame[frame_no].page_no = page_no;
    trans_table.frame[frame_no].flag = flag;
    return 0;
}

int mmu_switch(int pid) {
    char* code_base  = (void*) APPS_ENTRY;
    char* stack_base = (void*) DTIM_START;
    int code_npages  = APPS_SIZE / PAGE_SIZE;

    if (curr_vm_pid == -1)
        goto map_only;

    /* unmap curr_vm_pid from virtual address space */
    for (int i = 0; i < NFRAMES; i++) {
        if (INUSE(trans_table.frame[i])
            && trans_table.frame[i].pid == curr_vm_pid) {

            char* addr;
            int page_no = trans_table.frame[i].page_no;
            if (page_no < code_npages) {
                addr = code_base + page_no * PAGE_SIZE;
                cache_write(i, (void*)(addr));
            } else {
                addr = stack_base + (page_no - code_npages) * PAGE_SIZE;
                cache_write(i, (void*)(addr));
            }
            //INFO("Unmap(pid=%d, frame=%d, page=%d, vaddr=%.8x, paddr=%.8x)", curr_vm_pid, i, page_no, addr, &cache[i]);
        }
    }

 map_only:
    /* map pid to virtual address space */
    for (int i = 0; i < NFRAMES; i++) {
        if (INUSE(trans_table.frame[i])
            && trans_table.frame[i].pid == pid) {

            char *dst_addr, *src_addr = (char*)cache_read(i);
            int page_no = trans_table.frame[i].page_no;
            if (page_no < code_npages) {
                dst_addr = code_base + page_no * PAGE_SIZE;
                memcpy(dst_addr, src_addr, PAGE_SIZE);
            }
            else {
                dst_addr = stack_base + (page_no - code_npages) * PAGE_SIZE;
                memcpy(dst_addr, src_addr, PAGE_SIZE);
            }
            //INFO("Map(pid=%d, frame=%d, page=%d, vaddr=%.8x, paddr=%.8x)", pid, i, page_no, dst_addr, src_addr);
        }
    }

    curr_vm_pid = pid;
    return 0;
}

static int cache_evict() {
    int free_no = rand() % CACHED_NFRAMES;
    int frame_no = cache_frame_no[free_no];

    if (INUSE(trans_table.frame[frame_no])) {
        int nblocks = PAGE_SIZE / BLOCK_SIZE;
        disk_write(frame_no * nblocks, nblocks, (char*)(cache + free_no));
    }
    return free_no;
}

static int cache_read(int frame_no) {
    int free_no = -1;
    for (int i = 0; i < CACHED_NFRAMES; i++) {
        if (cache_frame_no[i] == frame_no)
            return (int)(cache + i);
        if (cache_frame_no[i] == -1 && free_no == -1)
            free_no = i;
    }

    if (free_no == -1)
        free_no = cache_evict();
    cache_frame_no[free_no] = frame_no;

    if (INUSE(trans_table.frame[frame_no])) {
        int nblocks = PAGE_SIZE / BLOCK_SIZE;
        disk_read(frame_no * nblocks, nblocks, (char*)(cache + free_no));
    }

    return (int)(cache + free_no);
}

static int cache_write(int frame_no, struct frame_t* src) {
    for (int i = 0; i < CACHED_NFRAMES; i++) {
        if (cache_frame_no[i] == frame_no) {
            memcpy(cache + i, src, PAGE_SIZE);
            return 0;
        }
    }

    int free_no = cache_evict();
    cache_frame_no[free_no] = frame_no;
    memcpy(cache + free_no, src, PAGE_SIZE);

    return 0;
}
