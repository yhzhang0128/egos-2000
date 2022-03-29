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

struct translation_table_t {
    struct {
        int pid;
        int page_no;
        int flag;
    } frame[NFRAMES];
} translate_table;

#define FRAME_INUSE(x) (translate_table.frame[x].flag & F_INUSE)

int curr_vm_pid;
int lookup_table[CACHED_NFRAMES];
char *cache = (void*)FRAME_CACHE_START;

static int cache_read(int frame_no);
static int cache_write(int frame_no, char* src);

int mmu_init() {
    curr_vm_pid = -1;
    memset(lookup_table, 0xff, sizeof(lookup_table));
    memset(&translate_table, 0, sizeof(translate_table));

    return 0;
}

int mmu_alloc(int* frame_no, int* cached_addr) {
    for (int i = 0; i < NFRAMES; i++) {
        if (!FRAME_INUSE(i)) {
            *frame_no = i;
            *cached_addr = cache_read(i);
            translate_table.frame[i].flag |= F_INUSE;
            return 0;
        }
    }
    return -1;
}

int mmu_free(int pid) {
    for (int i = 0; i < NFRAMES; i++) {
        if (translate_table.frame[i].pid == pid &&
            FRAME_INUSE(i)) {
            /* remove the mapping */
            translate_table.frame[i].pid = 0;
            translate_table.frame[i].page_no = 0;
            translate_table.frame[i].flag = 0;

            /* invalidate the cache */
            for (int j = 0; j < CACHED_NFRAMES; j++)
                if (lookup_table[j] == i)
                    lookup_table[j] = -1;
        }
    }
    return 0;
}

int mmu_map(int pid, int page_no, int frame_no, int flag) {
    if (flag != F_ALL)
        FATAL("Memory protection not implemented");
    
    if (!FRAME_INUSE(frame_no)) {
        INFO("Frame %d has not been allocated", frame_no);
        return -1;
    }

    translate_table.frame[frame_no].pid = pid;
    translate_table.frame[frame_no].page_no = page_no;
    translate_table.frame[frame_no].flag = flag;
    return 0;
}

int mmu_switch(int pid) {
    char *dst, *src;
    int code_top = APPS_SIZE / PAGE_SIZE;
    if (pid == curr_vm_pid) return 0;
    
    /* unmap curr_vm_pid from virtual address space */
    for (int i = 0; i < NFRAMES; i++)
        if (FRAME_INUSE(i) && translate_table.frame[i].pid == curr_vm_pid) {
            int page_no = translate_table.frame[i].page_no;
            src = (char*) ((page_no < code_top)? APPS_ENTRY : DTIM_START);
            cache_write(i, src + (page_no % code_top) * PAGE_SIZE);
            //INFO("Unmap(pid=%d, frame=%d, page=%d, vaddr=%.8x, paddr=%.8x)", curr_vm_pid, i, page_no, src + (page_no % code_top) * PAGE_SIZE, cache + i * PAGE_SIZE);
        }

    /* map pid to virtual address space */
    for (int i = 0; i < NFRAMES; i++)
        if (FRAME_INUSE(i) && translate_table.frame[i].pid == pid) {
            src = (char*)cache_read(i);
            int page_no = translate_table.frame[i].page_no;
            dst = (char*) ((page_no < code_top)? APPS_ENTRY : DTIM_START);
            memcpy(dst + (page_no % code_top) * PAGE_SIZE, src, PAGE_SIZE);
            //INFO("Map(pid=%d, frame=%d, page=%d, vaddr=%.8x, paddr=%.8x)", pid, i, page_no, dst + (page_no % code_top) * PAGE_SIZE, src);
        }

    curr_vm_pid = pid;
    return 0;
}

static int cache_evict() {
    int free_idx = rand() % CACHED_NFRAMES;
    int frame_no = lookup_table[free_idx];

    if (FRAME_INUSE(frame_no)) {
        int nblocks = PAGE_SIZE / BLOCK_SIZE;
        disk_write(frame_no * nblocks, nblocks, cache + PAGE_SIZE * free_idx);
    }

    return free_idx;
}

static int cache_read(int frame_no) {
    int free_idx = -1;
    for (int i = 0; i < CACHED_NFRAMES; i++) {
        if (lookup_table[i] == frame_no)
            return (int)(cache + PAGE_SIZE * i);
        if (lookup_table[i] == -1 && free_idx == -1)
            free_idx = i;
    }

    if (free_idx == -1)
        free_idx = cache_evict();
    lookup_table[free_idx] = frame_no;

    if (FRAME_INUSE(frame_no)) {
        int nblocks = PAGE_SIZE / BLOCK_SIZE;
        disk_read(frame_no * nblocks, nblocks, cache + PAGE_SIZE * free_idx);
    }

    return (int)(cache + PAGE_SIZE * free_idx);
}

static int cache_write(int frame_no, char* src) {
    for (int i = 0; i < CACHED_NFRAMES; i++) {
        if (lookup_table[i] == frame_no) {
            memcpy(cache + PAGE_SIZE * i, src, PAGE_SIZE);
            return 0;
        }
    }

    int free_idx = cache_evict();
    lookup_table[free_idx] = frame_no;
    memcpy(cache + PAGE_SIZE * free_idx, src, PAGE_SIZE);

    return 0;
}
