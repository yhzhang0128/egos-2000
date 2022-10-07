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

enum {
      QEMU_PAGE_TABLE,  /* By default, QEMU uses page table translation */
      ARTY_SOFTWARE_TLB  /* and the Arty board uses software translation */
};

#define NFRAMES             256
#define CACHED_NFRAMES      28    /* 32 - 4 */

/* definitions for translation table */
struct translation_table_t {
    struct {
        int pid;
        int page_no;
        int flag;
    } frame[NFRAMES];
} translate_table;

#define F_INUSE        0x1
#define FRAME_INUSE(x) (translate_table.frame[x].flag & F_INUSE)

/* definitions for frame cache */
int curr_vm_pid;
int lookup_table[CACHED_NFRAMES];
char *cache = (void*)FRAME_CACHE_START;

static struct earth* earth_local;
static int cache_read(int frame_no);
static int cache_write(int frame_no, char* src);

int arty_alloc(int* frame_no, int* cached_addr) {
    for (int i = 0; i < NFRAMES; i++)
        if (!FRAME_INUSE(i)) {
            *frame_no = i;
            *cached_addr = cache_read(i);
            translate_table.frame[i].flag |= F_INUSE;
            return 0;
        }
    FATAL("mmu_alloc: no more available frames");
}

int arty_free(int pid) {
    for (int i = 0; i < NFRAMES; i++)
        if (FRAME_INUSE(i) && translate_table.frame[i].pid == pid) {
            /* remove the mapping */
            translate_table.frame[i].pid = 0;
            translate_table.frame[i].page_no = 0;
            translate_table.frame[i].flag = 0;

            /* invalidate the cache */
            for (int j = 0; j < CACHED_NFRAMES; j++)
                if (lookup_table[j] == i) lookup_table[j] = -1;
        }
    return 0;
}

int arty_map(int pid, int page_no, int frame_no) {
    if (!FRAME_INUSE(frame_no)) FATAL("mmu_map: bad frame_no");

    translate_table.frame[frame_no].pid = pid;
    translate_table.frame[frame_no].page_no = page_no;
    return 0;
}

int arty_switch(int pid) {
    char *dst, *src;
    int code_top = APPS_SIZE / PAGE_SIZE;
    if (pid == curr_vm_pid) return 0;
    
    /* unmap curr_vm_pid from virtual address space */
    for (int i = 0; i < NFRAMES; i++)
        if (FRAME_INUSE(i) && translate_table.frame[i].pid == curr_vm_pid) {
            int page_no = translate_table.frame[i].page_no;
            src = (char*) ((page_no < code_top)? APPS_ENTRY : APPS_ARG);
            cache_write(i, src + (page_no % code_top) * PAGE_SIZE);
            /* INFO("Unmap(pid=%d, frame=%d, page=%d, vaddr=%.8x, paddr=%.8x)", curr_vm_pid, i, page_no, src + (page_no % code_top) * PAGE_SIZE, cache + i * PAGE_SIZE); */
        }

    /* map pid to virtual address space */
    for (int i = 0; i < NFRAMES; i++)
        if (FRAME_INUSE(i) && translate_table.frame[i].pid == pid) {
            src = (char*)cache_read(i);
            int page_no = translate_table.frame[i].page_no;
            dst = (char*) ((page_no < code_top)? APPS_ENTRY : APPS_ARG);
            memcpy(dst + (page_no % code_top) * PAGE_SIZE, src, PAGE_SIZE);
            /* INFO("Map(pid=%d, frame=%d, page=%d, vaddr=%.8x, paddr=%.8x)", pid, i, page_no, dst + (page_no % code_top) * PAGE_SIZE, src); */
        }

    curr_vm_pid = pid;
    return 0;
}

static int cache_evict() {
    int idx = rand() % CACHED_NFRAMES;
    int frame_no = lookup_table[idx];

    if (FRAME_INUSE(frame_no)) {
        int nblocks = PAGE_SIZE / BLOCK_SIZE;
        earth->disk_write(frame_no * nblocks, nblocks, cache + PAGE_SIZE * idx);
    }

    return idx;
}

static int cache_read(int frame_no) {
    int free_idx = -1;
    for (int i = 0; i < CACHED_NFRAMES; i++) {
        if (lookup_table[i] == frame_no)
            return (int)(cache + PAGE_SIZE * i);
        if (lookup_table[i] == -1 && free_idx == -1)
            free_idx = i;
    }

    if (free_idx == -1) free_idx = cache_evict();
    lookup_table[free_idx] = frame_no;

    if (FRAME_INUSE(frame_no)) {
        int nblocks = PAGE_SIZE / BLOCK_SIZE;
        earth->disk_read(frame_no * nblocks, nblocks, cache + PAGE_SIZE * free_idx);
    }

    return (int)(cache + PAGE_SIZE * free_idx);
}

static int cache_write(int frame_no, char* src) {
    for (int i = 0; i < CACHED_NFRAMES; i++)
        if (lookup_table[i] == frame_no) {
            memcpy(cache + PAGE_SIZE * i, src, PAGE_SIZE);
            return 0;
        }

    int free_idx = cache_evict();
    lookup_table[free_idx] = frame_no;
    memcpy(cache + PAGE_SIZE * free_idx, src, PAGE_SIZE);

    return 0;
}

int qemu_alloc(int* frame_no, int* cached_addr) {
    arty_alloc(frame_no, cached_addr);
}
int qemu_free(int pid) { arty_free(pid); }
int qemu_map(int pid, int page_no, int frame_no) {
    arty_map(pid, page_no, frame_no);
}
int qemu_switch(int pid) { arty_switch(pid); }

static int machine, tmp;
void machine_detect(int id) {
    machine = ARTY_SOFTWARE_TLB;
    /* Skip the illegal store instruction */
    asm("csrr %0, mepc" : "=r"(tmp));
    asm("csrw mepc, %0" ::"r"(tmp + 4));
}

void mmu_init(struct earth* _earth) {
    earth = _earth;
    earth->excp_register(machine_detect);
    /* This memory access triggers an exception on Arty, but not QEMU */
    *(int*)(0x1000) = 1;
    earth->excp_register(NULL);

    if (machine == ARTY_SOFTWARE_TLB) {
        earth->tty_info("Use software translation for Arty");
        earth->mmu_free = arty_free;
        earth->mmu_alloc = arty_alloc;
        earth->mmu_map = arty_map;
        earth->mmu_switch = arty_switch;
    } else {
        earth->tty_info("Use page table translation for QEMU");
        earth->mmu_free = qemu_free;
        earth->mmu_alloc = qemu_alloc;
        earth->mmu_map = qemu_map;
        earth->mmu_switch = qemu_switch;
    }

    curr_vm_pid = -1;
    memset(lookup_table, 0xFF, sizeof(lookup_table));
}
