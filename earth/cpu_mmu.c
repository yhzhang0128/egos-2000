/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: memory management unit (MMU)
 * This file implements 2 translation mechanisms:
 *     page table and software TLB.
 * The Arty board uses software TLB and QEMU uses both.
 * This file also implements a paging device and MMU initialization.
 */

#include "egos.h"
#include "disk.h"
#include <stdlib.h>
#include <string.h>

/* Interface of a 1MB (256*4KB) paging device
 * On QEMU, it is simply the first 1MB of the memory
 * On Arty board, it is the first 1MB of the microSD card
 *   and some frames in the paging device are cached in memory
 */
#define NFRAMES 256
char* paging_read(int frame_id);
int paging_write(int frame_id, int page_no);
int paging_invalidate_cache(int frame_id);

/* Allocation and free of physical frames */
struct frame_mapping {
    int use;     /* Is the frame allocated? */
    int pid;     /* Which process owns the frame? */
    int page_no; /* Which virtual page is the frame mapped to? */
} table[NFRAMES];

int mmu_alloc(int* frame_id, void** cached_addr) {
    for (int i = 0; i < NFRAMES; i++)
        if (!table[i].use) {
            *frame_id = i;
            *cached_addr = paging_read(i);
            table[i].use = 1;
            return 0;
        }
    FATAL("mmu_alloc: no more available frames");
}

int mmu_free(int pid) {
    for (int i = 0; i < NFRAMES; i++)
        if (table[i].use && table[i].pid == pid) {
            paging_invalidate_cache(i);
            memset(&table[i], 0, sizeof(struct frame_mapping));
        }
}

/* Software TLB Translation */
int soft_mmu_map(int pid, int page_no, int frame_id) {
    if (!table[frame_id].use) FATAL("mmu_map: bad frame_id");
    
    table[frame_id].pid = pid;
    table[frame_id].page_no = page_no;
}

int soft_mmu_switch(int pid) {
    static int curr_vm_pid = -1;
    if (pid == curr_vm_pid) return 0;

    /* Unmap curr_vm_pid from the user address space */
    for (int i = 0; i < NFRAMES; i++)
        if (table[i].use && table[i].pid == curr_vm_pid)
            paging_write(i, table[i].page_no);

    /* Map pid to the user address space */
    for (int i = 0; i < NFRAMES; i++)
        if (table[i].use && table[i].pid == pid)
            memcpy((void*)(table[i].page_no << 12), paging_read(i), PAGE_SIZE);

    curr_vm_pid = pid;
}

/* Page Table Translation
 *
 * The code below creates an identity mapping using RISC-V Sv32.
 * mmu_map() and mmu_switch() are still implemented by software TLB.
 *
 * Rewriting mmu_map() and mmu_switch() with Sv32 is left to students
 * as a course project. After this project, every process should have
 * its own set of page tables. mmu_map() will modify entries in these
 * page tables and mmu_switch() will modify the satp register, i.e., 
 * the page table base register.
 */

#define FLAG_VALID_RWX 0xF
#define FLAG_NEXT_LEVEL 0x1
static unsigned int frame_id, *root, *leaf;

void setup_identity_region(unsigned int addr, int npages) {
    /* Allocate the leaf page table */
    earth->mmu_alloc(&frame_id, (void**)&leaf);
    memset(leaf, 0, PAGE_SIZE);

    /* Setup the entry in the root page table */
    int vpn1 = addr >> 22;
    root[vpn1] = ((unsigned int)leaf >> 2) | FLAG_NEXT_LEVEL;

    /* Setup the entries in the leaf page table */
    int vpn0 = (addr >> 12) & 0x3FF;
    for (int i = 0; i < npages; i++)
        leaf[vpn0 + i] = ((addr + i * PAGE_SIZE) >> 2) | FLAG_VALID_RWX;
}

void pagetable_identity_mapping() {
    /* Allocate the root page table and set the page table base (satp) */
    earth->mmu_alloc(&frame_id, (void**)&root);
    memset(root, 0, PAGE_SIZE);
    /* Set the (1 << 31) bit of satp to enable Sv32 translation */
    asm("csrw satp, %0" ::"r"(((unsigned int)root >> 12) | (1 << 31)));

    /* Allocate the leaf page tables */
    setup_identity_region(0x02000000, 16);   /* CLINT */
    setup_identity_region(0x10013000, 1);    /* UART0 */
    setup_identity_region(0x20400000, 1024); /* boot ROM */
    setup_identity_region(0x20800000, 1024); /* disk image */
    setup_identity_region(0x08000000, 8);    /* ITIM memory */
    setup_identity_region(0x80000000, 1024); /* DTIM memory */

    /* Translation will start when the earth main() invokes mret so that the processor enters supervisor mode from machine mode */
}

/* A paging device with in-memory caching
 *
 * For QEMU, 256 physical frames start at address FRAME_CACHE_START.
 * For Arty, 28 physical frames are cached at address FRAME_CACHE_START
 * and 256 frames start at the beginning of the microSD card.
 */

#define ARTY_CACHED_NFRAMES 28
#define NBLOCKS_PER_PAGE PAGE_SIZE / BLOCK_SIZE  /* 4KB / 512B == 8 */

int cache_slots[ARTY_CACHED_NFRAMES];
char *pages_start = (void*)FRAME_CACHE_START;

int paging_evict_cache() {
    /* Randomly select a cache slot to evict */
    int idx = rand() % ARTY_CACHED_NFRAMES;
    int frame_id = cache_slots[idx];

    if (table[frame_id].use)
        earth->disk_write(frame_id * NBLOCKS_PER_PAGE, NBLOCKS_PER_PAGE, pages_start + PAGE_SIZE * idx);

    return idx;
}

int paging_invalidate_cache(int frame_id) {
    for (int j = 0; j < ARTY_CACHED_NFRAMES; j++)
        if (cache_slots[j] == frame_id) cache_slots[j] = -1;
}

char* paging_read(int frame_id) {
    if (earth->platform == QEMU) return pages_start + frame_id * PAGE_SIZE;

    int free_idx = -1;
    for (int i = 0; i < ARTY_CACHED_NFRAMES; i++) {
        if (cache_slots[i] == -1 && free_idx == -1) free_idx = i;
        if (cache_slots[i] == frame_id) return pages_start + PAGE_SIZE * i;
    }

    if (free_idx == -1) free_idx = paging_evict_cache();
    cache_slots[free_idx] = frame_id;

    if (table[frame_id].use)
        earth->disk_read(frame_id * NBLOCKS_PER_PAGE, NBLOCKS_PER_PAGE, pages_start + PAGE_SIZE * free_idx);

    return pages_start + PAGE_SIZE * free_idx;
}

int paging_write(int frame_id, int page_no) {
    char* src = (void*)(page_no << 12);
    if (earth->platform == QEMU) {
        memcpy(pages_start + frame_id * PAGE_SIZE, src, PAGE_SIZE);
        return 0;
    }

    for (int i = 0; i < ARTY_CACHED_NFRAMES; i++)
        if (cache_slots[i] == frame_id) {
            memcpy(pages_start + PAGE_SIZE * i, src, PAGE_SIZE) != NULL;
            return 0;
        }

    int free_idx = paging_evict_cache();
    cache_slots[free_idx] = frame_id;
    memcpy(pages_start + PAGE_SIZE * free_idx, src, PAGE_SIZE);
}

/* MMU Initialization */
void platform_detect(int id) {
    earth->platform = ARTY;
    /* Skip the illegal store instruction */
    int mepc;
    asm("csrr %0, mepc" : "=r"(mepc));
    asm("csrw mepc, %0" ::"r"(mepc + 4));
}

void mmu_init() {
    /* Choose memory translation mechanism */
    CRITICAL("Choose a memory translation mechanism:");
    printf("  Enter 0: page tables  (QEMU)\r\n");
    printf("  Enter 1: software TLB (QEMU or Arty board)\r\n");

    char buf[2];
    for (buf[0] = 0; buf[0] != '0' && buf[0] != '1'; earth->tty_read(buf, 2));
    INFO("%s translation is chosen", (buf[0] == '0')? "Page table" : "Software");

    /* Check whether the hardware platform supports supervisor mode */
    earth->platform = QEMU;
    earth->excp_register(platform_detect);
    /* This memory access triggers an exception on Arty, but not QEMU */
    *(int*)(0x1000) = 1;
    earth->excp_register(NULL);
    if (earth->platform == ARTY && buf[0] == '0')
        FATAL("Arty board doesn't support page tables (supervisor mode).");

    /* Initialize MMU interface functions */
    earth->mmu_free = mmu_free;
    earth->mmu_alloc = mmu_alloc;
    earth->mmu_map = soft_mmu_map;
    earth->mmu_switch = soft_mmu_switch;

    memset(cache_slots, 0xFF, sizeof(cache_slots));
    if (buf[0] == '0') pagetable_identity_mapping();
}
