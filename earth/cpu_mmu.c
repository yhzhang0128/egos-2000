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

#include "egos.h"
#include "earth.h"
#include <stdlib.h>

/* Implementation of Software TLB Translation
 *
 * For the Arty board, there are 256 physical frames at the start of
 * the SD card and 28 are cached in memory (i.e., the DTIM cache).
 * For QEMU, the 256 physical frames are all in memory.
 */

#define NFRAMES             256
#define ARTY_CACHED_NFRAMES 28  /* 32 - 4, 4 pages for 2 stacks */

/* the software translation table */
struct {
    int pid, page_no, use;
} frame[NFRAMES];

int curr_vm_pid;
static int cache_read(int frame_no);
static int cache_write(int frame_no, char* src);
static int cache_invalidate(int frame_no);

int soft_mmu_alloc(int* frame_no, int* cached_addr) {
    for (int i = 0; i < NFRAMES; i++)
        if (!frame[i].use) {
            *frame_no = i;
            *cached_addr = cache_read(i);
            frame[i].use = 1;
            return 0;
        }
    FATAL("mmu_alloc: no more available frames");
}

int soft_mmu_free(int pid) {
    for (int i = 0; i < NFRAMES; i++)
        if (frame[i].use && frame[i].pid == pid) {
            /* remove the mapping */
            memset(&frame[i], 0, sizeof(int) * 3);
            cache_invalidate(i);
        }
    return 0;
}

int soft_mmu_map(int pid, int page_no, int frame_no) {
    if (!frame[frame_no].use) FATAL("mmu_map: bad frame_no");

    frame[frame_no].pid = pid;
    frame[frame_no].page_no = page_no;
    return 0;
}

int soft_mmu_switch(int pid) {
    char *dst, *src;
    int code_top = APPS_SIZE / PAGE_SIZE;
    if (pid == curr_vm_pid) return 0;
    
    /* unmap curr_vm_pid from virtual address space */
    for (int i = 0; i < NFRAMES; i++)
        if (frame[i].use && frame[i].pid == curr_vm_pid) {
            int page_no = frame[i].page_no;
            src = (char*) ((page_no < code_top)? APPS_ENTRY : APPS_ARG);
            cache_write(i, src + (page_no % code_top) * PAGE_SIZE);
            /* INFO("Unmap(pid=%d, frame=%d, page=%d, vaddr=%.8x, paddr=%.8x)", curr_vm_pid, i, page_no, src + (page_no % code_top) * PAGE_SIZE, cache + i * PAGE_SIZE); */
        }

    /* map pid to virtual address space */
    for (int i = 0; i < NFRAMES; i++)
        if (frame[i].use && frame[i].pid == pid) {
            src = (char*)cache_read(i);
            int page_no = frame[i].page_no;
            dst = (char*) ((page_no < code_top)? APPS_ENTRY : APPS_ARG);
            memcpy(dst + (page_no % code_top) * PAGE_SIZE, src, PAGE_SIZE);
            /* INFO("Map(pid=%d, frame=%d, page=%d, vaddr=%.8x, paddr=%.8x)", pid, i, page_no, dst + (page_no % code_top) * PAGE_SIZE, src); */
        }

    curr_vm_pid = pid;
    return 0;
}

/* Implementation of Page Table Translation
 *
 * The code below creates a simple identity mapping using page tables.
 * mmu_map() and mmu_switch() are still implemented by software TLB.
 *
 * Using page tables for mmu_map() and mmu_switch() is left to students
 * as a course project.
 */

unsigned int frame_no, root;
void setup_identity_region(unsigned int addr, int npages) {
    /* Allocate leaf page table */
    unsigned int leaf;
    earth->mmu_alloc(&frame_no, &leaf);
    memset((void*)leaf, 0, PAGE_SIZE);

    /* Setup the mapping in the root page table */
    int vpn1 = addr >> 22;
    ((int*)root)[vpn1] = (leaf >> 2) | 0x1;

    /* Setup the mapping in the leaf page table */
    int vpn0 = (addr >> 12) & 0x3FF;
    for (int i = 0; i < npages; i++) {
        ((int*)leaf)[vpn0 + i] = ((addr + i * PAGE_SIZE) >> 2) | 0xF;
    }
}

void create_identity_mapping() {
    /* Allocate the root page table and set the page table base */
    earth->mmu_alloc(&frame_no, &root);
    memset((void*)root, 0, PAGE_SIZE);
    asm("csrw satp, %0" ::"r"(((root >> 12) & 0xFFFFF) | (1 << 31)));

    setup_identity_region(0x02000000, 16);   /* CLINT */
    setup_identity_region(0x10013000, 1);    /* UART0 */
    setup_identity_region(0x20400000, 1024); /* boot ROM */
    setup_identity_region(0x20800000, 1024); /* disk image */
    setup_identity_region(0x08000000, 8);    /* ITIM memory */
    setup_identity_region(0x80000000, 1024); /* DTIM memory */
}

/* Implementation of MMU Initialization and a Cache
 *
 * Detect whether egos-2000 is running on QEMU or the Arty board.
 * Choose the memory translation mechanism accordingly.
 */

int lookup_table[ARTY_CACHED_NFRAMES];
char *cache = (void*)FRAME_CACHE_START;

static int mepc, mstatus;
void machine_detect(int id) {
    earth->platform = ARTY;
    /* Skip the illegal store instruction */
    asm("csrr %0, mepc" : "=r"(mepc));
    asm("csrw mepc, %0" ::"r"(mepc + 4));
}

void mmu_init(struct earth* _earth) {
    earth = _earth;
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
        earth->tty_info("Use software + page-table translation for QEMU");

        create_identity_mapping();
        /* Later enter the grass layer in supervisor mode */
        asm("csrr %0, mstatus" : "=r"(mstatus));
        asm("csrw mstatus, %0" ::"r"((mstatus & ~(3 << 11)) | (1 << 11) ));
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

    if (frame[frame_no].use) {
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

    if (frame[frame_no].use) {
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

    return 0;
}
