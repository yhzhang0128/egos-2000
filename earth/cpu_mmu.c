/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: memory management unit (MMU)
 * This file implements 2 translation mechanisms:
 *     page table translation and software TLB translation.
 * The Arty board uses software TLB translation and QEMU uses both.
 */

#include "earth.h"
#include <stdlib.h>

/* Implementation of Software TLB Translation
 *
 * There are 256 physical frames at the start of the disk (e.g., SD
 * card for Arty) and 28 are cached in memory (i.e., the DTIM cache).
 */

#define NFRAMES             256
#define ARTY_CACHED_NFRAMES 28  /* 32 - 4, 4 pages for 2 stacks */

/* the software translation table */
struct {
    int pid, page_no, use;
} table[NFRAMES];

int soft_mmu_map(int pid, int page_no, int frame_no) {
    if (!table[frame_no].use) FATAL("mmu_map: bad frame_no");
    
    table[frame_no].pid = pid;
    table[frame_no].page_no = page_no;
}

int curr_vm_pid;
static int cache_read(int frame_no);
static int cache_write(int frame_no, char* src);
static int cache_invalidate(int frame_no);

int soft_mmu_switch(int pid) {
    if (pid == curr_vm_pid) return 0;

    char *dst, *src;
    int code_top = APPS_SIZE / PAGE_SIZE;
    /* unmap curr_vm_pid from virtual address space */
    for (int i = 0; i < NFRAMES; i++)
        if (table[i].use && table[i].pid == curr_vm_pid) {
            int page_no = table[i].page_no;
            src = (char*) ((page_no < code_top)? APPS_ENTRY : APPS_ARG);
            cache_write(i, src + (page_no % code_top) * PAGE_SIZE);
        }

    /* map pid to virtual address space */
    for (int i = 0; i < NFRAMES; i++)
        if (table[i].use && table[i].pid == pid) {
            src = (char*)cache_read(i);
            int page_no = table[i].page_no;
            dst = (char*) ((page_no < code_top)? APPS_ENTRY : APPS_ARG);
            memcpy(dst + (page_no % code_top) * PAGE_SIZE, src, PAGE_SIZE);
        }

    curr_vm_pid = pid;
}

int soft_mmu_alloc(int* frame_no, int* cached_addr) {
    for (int i = 0; i < NFRAMES; i++)
        if (!table[i].use) {
            *frame_no = i;
            table[i].use = 1;
            *cached_addr = cache_read(i);
            return 0;
        }
    FATAL("mmu_alloc: no more available frames");
}

int soft_mmu_free(int pid) {
    for (int i = 0; i < NFRAMES; i++)
        if (table[i].use && table[i].pid == pid) {
            memset(&table[i], 0, sizeof(int) * 3);
            cache_invalidate(i);
        }
}

/* Implementation of Page Table Translation
 *
 * The code below creates a simple identity mapping using page tables.
 * mmu_map() and mmu_switch() are still implemented by software TLB.
 *
 * Using page tables for mmu_map() and mmu_switch() is left to students
 * as a course project.
 */

unsigned int frame_no, root, leaf;
void setup_identity_region(unsigned int addr, int npages) {
    /* Allocate leaf page table */
    earth->mmu_alloc(&frame_no, &leaf);
    memset((void*)leaf, 0, PAGE_SIZE);

    /* Setup the mapping in the root page table */
    int vpn1 = addr >> 22;
    ((int*)root)[vpn1] = (leaf >> 2) | 0x1;

    /* Setup the mapping in the leaf page table */
    int vpn0 = (addr >> 12) & 0x3FF;
    for (int i = 0; i < npages; i++)
        ((int*)leaf)[vpn0 + i] = ((addr + i * PAGE_SIZE) >> 2) | 0xF;
}

void pagetable_identity_mapping() {
    /* Allocate the root page table and set the page table base */
    earth->mmu_alloc(&frame_no, &root);
    memset((void*)root, 0, PAGE_SIZE);
    asm("csrw satp, %0" ::"r"(((root >> 12) & 0xFFFFF) | (1 << 31)));

    /* Allocate the leaf page tables */
    setup_identity_region(0x02000000, 16);   /* CLINT */
    setup_identity_region(0x10013000, 1);    /* UART0 */
    setup_identity_region(0x20400000, 1024); /* boot ROM */
    setup_identity_region(0x20800000, 1024); /* disk image */
    setup_identity_region(0x08000000, 8);    /* ITIM memory */
    setup_identity_region(0x80000000, 1024); /* DTIM memory */
}

/* Implementation of MMU Initialization and a Paging Cache
 *
 * Detect whether egos-2000 is running on QEMU or the Arty board.
 * And choose the memory translation mechanism accordingly.
 */

void machine_detect(int id) {
    earth->platform = ARTY;
    /* Skip the illegal store instruction */
    int mepc;
    asm("csrr %0, mepc" : "=r"(mepc));
    asm("csrw mepc, %0" ::"r"(mepc + 4));
}

int lookup_table[ARTY_CACHED_NFRAMES];
char *cache = (void*)FRAME_CACHE_START;

void mmu_init() {
    earth->platform = QEMU;
    earth->excp_register(machine_detect);
    /* This memory access triggers an exception on Arty, but not QEMU */
    *(int*)(0x1000) = 1;
    earth->excp_register(NULL);

    earth->mmu_alloc = soft_mmu_alloc;
    earth->mmu_map = soft_mmu_map;
    earth->mmu_switch = soft_mmu_switch;
    earth->mmu_free = soft_mmu_free;

    if (earth->platform == ARTY) {
        earth->tty_info("Use software translation for Arty");
    } else {
        pagetable_identity_mapping();
        earth->tty_info("Use software + page-table translation for QEMU");
    }

    curr_vm_pid = -1;
    memset(lookup_table, 0xFF, sizeof(lookup_table));
}

static int cache_invalidate(int frame_no) {
    for (int j = 0; j < ARTY_CACHED_NFRAMES; j++)
        if (lookup_table[j] == frame_no) lookup_table[j] = -1;
}

static int cache_evict() {
    int idx = rand() % ARTY_CACHED_NFRAMES;
    int frame_no = lookup_table[idx];

    if (table[frame_no].use) {
        int nblocks = PAGE_SIZE / BLOCK_SIZE;
        earth->disk_write(frame_no * nblocks, nblocks, cache + PAGE_SIZE * idx);
    }

    return idx;
}

static int cache_read(int frame_no) {
    if (earth->platform == QEMU)
        return (int)(cache + frame_no * PAGE_SIZE);

    int free_idx = -1;
    for (int i = 0; i < ARTY_CACHED_NFRAMES; i++) {
        if (lookup_table[i] == frame_no)
            return (int)(cache + PAGE_SIZE * i);
        if (lookup_table[i] == -1 && free_idx == -1) free_idx = i;
    }

    if (free_idx == -1) free_idx = cache_evict();
    lookup_table[free_idx] = frame_no;

    if (table[frame_no].use) {
        int nblocks = PAGE_SIZE / BLOCK_SIZE;
        earth->disk_read(frame_no * nblocks, nblocks, cache + PAGE_SIZE * free_idx);
    }

    return (int)(cache + PAGE_SIZE * free_idx);
}

static int cache_write(int frame_no, char* src) {
    if (earth->platform == QEMU) {
        memcpy(cache + frame_no * PAGE_SIZE, src, PAGE_SIZE);
        return 0;
    }

    for (int i = 0; i < ARTY_CACHED_NFRAMES; i++)
        if (lookup_table[i] == frame_no)
            return memcpy(cache + PAGE_SIZE * i, src, PAGE_SIZE) != NULL;

    int free_idx = cache_evict();
    lookup_table[free_idx] = frame_no;
    memcpy(cache + PAGE_SIZE * free_idx, src, PAGE_SIZE);
}
