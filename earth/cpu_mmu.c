/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: memory management unit (MMU)
 * This file implements 2 translation mechanisms:
 *     page table translation and software TLB translation.
 * The Arty board uses software TLB translation and QEMU uses both.
 * This file also implements a paging device and initialization of MMU.
 */

#include "earth.h"
#include <stdlib.h>

/* Implementation of Software TLB Translation
 *
 * There are 256 physical frames (i.e., pages) in a paging device. 
 * The paging device provides read, write and cache_invalidate interfaces.
 */

/* the paging device */
#define NFRAMES 256
int paging_read(int frame_no);
int paging_write(int frame_no, char* src);
int paging_invalidate_cache(int frame_no);

/* the software translation table */
struct frame_mapping {
    int pid;
    int page_no;
    int use;
} table[NFRAMES];
#define PAGENO_TO_ADDR(x) (void*)(table[x].page_no << 12)

int soft_mmu_map(int pid, int page_no, int frame_no) {
    if (!table[frame_no].use) FATAL("mmu_map: bad frame_no");
    
    table[frame_no].pid = pid;
    table[frame_no].page_no = page_no;
}

int soft_mmu_switch(int pid) {
    static int curr_vm_pid = -1;
    if (pid == curr_vm_pid) return 0;

    /* Unmap curr_vm_pid from the user address space */
    for (int i = 0; i < NFRAMES; i++)
        if (table[i].use && table[i].pid == curr_vm_pid)
            paging_write(i, PAGENO_TO_ADDR(i));

    /* Map pid to the user address space */
    for (int i = 0; i < NFRAMES; i++)
        if (table[i].use && table[i].pid == pid)
            memcpy(PAGENO_TO_ADDR(i), (void*)paging_read(i), PAGE_SIZE);

    curr_vm_pid = pid;
}

int soft_mmu_alloc(int* frame_no, int* cached_addr) {
    for (int i = 0; i < NFRAMES; i++)
        if (!table[i].use) {
            *frame_no = i;
            *cached_addr = paging_read(i);
            table[i].use = 1;
            return 0;
        }
    FATAL("mmu_alloc: no more available frames");
}

int soft_mmu_free(int pid) {
    for (int i = 0; i < NFRAMES; i++)
        if (table[i].use && table[i].pid == pid) {
            paging_invalidate_cache(i);
            memset(&table[i], 0, sizeof(struct frame_mapping));
        }
}

/* Implementation of Page Table Translation
 *
 * The code below creates an identity mapping using page tables.
 * mmu_map() and mmu_switch() are still implemented by software TLB.
 *
 * Using page tables for mmu_map() and mmu_switch() is left to students
 * as a course project. After this project, each process in the grass
 * kernel should have its own set of page tables.
 */

#define FLAG_VALID_RWX 0xF
#define FLAG_NEXT_LEVEL 0x1
static unsigned int frame_no, root, leaf;

void setup_identity_region(unsigned int addr, int npages) {
    /* Allocate the leaf page table */
    earth->mmu_alloc(&frame_no, &leaf);
    memset((void*)leaf, 0, PAGE_SIZE);

    /* Setup the entry in the root page table */
    int vpn1 = addr >> 22;
    ((int*)root)[vpn1] = (leaf >> 2) | FLAG_NEXT_LEVEL;

    /* Setup the entries in the leaf page table */
    int vpn0 = (addr >> 12) & 0x3FF;
    for (int i = 0; i < npages; i++)
        ((int*)leaf)[vpn0 + i] = ((addr + i * PAGE_SIZE) >> 2) | FLAG_VALID_RWX;
}

void pagetable_identity_mapping() {
    /* Allocate the root page table and set the page table base CSR */
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

/* Implementation of a Paging Device
 *
 * For QEMU, there are 256 frames starting at address FRAME_CACHE_START.
 * For Arty, there are 28 frames cached at address FRAME_CACHE_START 
 * and 256 frames at the beginning of the disk, i.e., the microSD card.
 */

#define ARTY_CACHED_NFRAMES 28  /* 32 - 4, 4 pages for 2 stacks */
int cache_slot[ARTY_CACHED_NFRAMES];
char *page_start = (void*)FRAME_CACHE_START;

int paging_evict_cache() {
    int idx = rand() % ARTY_CACHED_NFRAMES;
    int frame_no = cache_slot[idx];

    if (table[frame_no].use) {
        int nblocks = PAGE_SIZE / BLOCK_SIZE;
        earth->disk_write(frame_no * nblocks, nblocks, page_start + PAGE_SIZE * idx);
    }

    return idx;
}

int paging_invalidate_cache(int frame_no) {
    for (int j = 0; j < ARTY_CACHED_NFRAMES; j++)
        if (cache_slot[j] == frame_no) cache_slot[j] = -1;
}

void paging_init() { memset(cache_slot, 0xFF, sizeof(cache_slot)); }

int paging_read(int frame_no) {
    if (earth->platform == QEMU)
        return (int)(page_start + frame_no * PAGE_SIZE);

    int free_idx = -1;
    for (int i = 0; i < ARTY_CACHED_NFRAMES; i++) {
        if (cache_slot[i] == frame_no)
            return (int)(page_start + PAGE_SIZE * i);
        if (cache_slot[i] == -1 && free_idx == -1) free_idx = i;
    }

    if (free_idx == -1) free_idx = paging_evict_cache();
    cache_slot[free_idx] = frame_no;

    if (table[frame_no].use) {
        int nblocks = PAGE_SIZE / BLOCK_SIZE;
        earth->disk_read(frame_no * nblocks, nblocks, page_start + PAGE_SIZE * free_idx);
    }

    return (int)(page_start + PAGE_SIZE * free_idx);
}

int paging_write(int frame_no, char* src) {
    if (earth->platform == QEMU) {
        memcpy(page_start + frame_no * PAGE_SIZE, src, PAGE_SIZE);
        return 0;
    }

    for (int i = 0; i < ARTY_CACHED_NFRAMES; i++)
        if (cache_slot[i] == frame_no)
            return memcpy(page_start + PAGE_SIZE * i, src, PAGE_SIZE) != NULL;

    int free_idx = paging_evict_cache();
    cache_slot[free_idx] = frame_no;
    memcpy(page_start + PAGE_SIZE * free_idx, src, PAGE_SIZE);
}

/* Implementation of MMU Initialization
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
        paging_init();
        earth->tty_info("Use software translation for Arty");
    } else {
        pagetable_identity_mapping();
        earth->tty_info("Use software + page table translation for QEMU");
    }
}
