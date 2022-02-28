/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: abstractions of the CPU memory management unit (MMU);
 * there are 256 physical frames at the start of the SD card and 20 of
 * them are cached in the memory (more precisely, in the CPU L1 cache);
 * the first 12 pages in memory are used as the virtual memory address
 * space for a process; there are totally 20 + 12 = 32 pages used by
 * kernel/user processes; other pages are used by earth and grass;
 * find more information in the documents
 */

#include "egos.h"
#include "earth.h"

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

int mmu_alloc(int* frame_no, int* addr) {
    for (int i = 0; i < NFRAMES; i++) {
        if (!INUSE(trans_table.frame[i])) {
            *frame_no = i;
            *addr = cache_read(i);
            USE(trans_table.frame[i]);
            return 0;
        }
    }
    return -1;
}

int mmu_map(int pid, int page_no, int frame_no, int flag) {
    if (flag != F_ALL)
        FATAL("Memory protection not implemented in earth");
    
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
            INFO("Unmap(pid=%d, frame=%d, page=%d, vaddr=%.8x, paddr=%.8x)", curr_vm_pid, i, page_no, addr, &cache[i]);
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
            INFO("Map(pid=%d, frame=%d, page=%d, vaddr=%.8x, paddr=%.8x)", pid, i, page_no, dst_addr, src_addr);
        }
    }

    curr_vm_pid = pid;
    return 0;
}

static int cache_read(int frame_no) {
    for (int i = 0; i < CACHED_NFRAMES; i++) {
        if (cache_frame_no[i] == frame_no)
            return (int)(cache + i);
    }

    int free_no = -1;
    for (int i = 0; i < CACHED_NFRAMES; i++) {
        if (cache_frame_no[i] == -1 && free_no == -1) {
            free_no = i;
            break;
        }
    }

    if (free_no != -1) {
        cache_frame_no[free_no] = frame_no;
        int group = PAGE_SIZE / BLOCK_SIZE;
        disk_read(free_no * group, group, (char*)(cache + free_no));

        return (int)(cache + free_no);
    } else {
        FATAL("Cache is full in cache_read(), cpu_mmu.c");
    }
}

static int cache_write(int frame_no, struct frame_t* src) {
    int found_no = -1;
    for (int i = 0; i < CACHED_NFRAMES; i++) {
        if (cache_frame_no[i] == frame_no) {
            memcpy(cache + i, src, PAGE_SIZE);
            return 0;
        }
    }

    FATAL("Cache is full in cache_write(), cpu_mmu.c");
}
