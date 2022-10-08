/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: abstractions of the memory management unit (MMU);
 * By default, QEMU uses page table translation and the Arty board
 * uses software TLB translation.
 */

#include "egos.h"
#include "earth.h"
#include <stdlib.h>

/* Implementation of Software TLB Translation for Arty
 *
 * There are 256 physical frames at the start of the SD card and 28 of
 * them are cached in the memory (more precisely, in the DTIM cache).
 */

#define NFRAMES             256
#define CACHED_NFRAMES      28    /* 32 - 4 */

/* definition of the software translation table */
struct translation_table_t {
    struct {
        int pid, page_no, flag;
    } frame[NFRAMES];
} translate_table;

#define F_INUSE        0x1
#define FRAME_INUSE(x) (translate_table.frame[x].flag & F_INUSE)

/* definitions of the frame cache (for paging to disk) */
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
            memset(&translate_table.frame[i], 0, sizeof(int) * 3);
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
        if (lookup_table[i] == -1 && free_idx == -1) free_idx = i;
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
        if (lookup_table[i] == frame_no)
            return memcpy(cache + PAGE_SIZE * i, src, PAGE_SIZE) != NULL;

    int free_idx = cache_evict();
    lookup_table[free_idx] = frame_no;
    memcpy(cache + PAGE_SIZE * free_idx, src, PAGE_SIZE);

    return 0;
}


/* Implementation of Page Table Translation for QEMU
 *
 *
 *
 */

int next_free_page_no;
#define QEMU_NPAGES 1024
char *first_page = (void*)FRAME_CACHE_START;

int qemu_alloc(int* frame_no, int* cached_addr) {
    *frame_no = next_free_page_no++;
    *cached_addr = (int)(first_page + PAGE_SIZE * (*frame_no));
    if (*frame_no == QEMU_NPAGES) FATAL("qemu_alloc: no more free pages");
    translate_table.frame[*frame_no].flag |= F_INUSE;
}

int qemu_free(int pid) { /* Ommitted for simplicity */ }

int qemu_map(int pid, int page_no, int frame_no) {
    translate_table.frame[frame_no].pid = pid;
    translate_table.frame[frame_no].page_no = page_no;
    return 0;
}

int qemu_switch(int pid) {
    char *dst, *src;
    int code_top = APPS_SIZE / PAGE_SIZE;
    if (pid == curr_vm_pid) return 0;

    /* unmap curr_vm_pid from virtual address space */
    for (int i = 0; i < NFRAMES; i++)
        if (FRAME_INUSE(i) && translate_table.frame[i].pid == curr_vm_pid) {
            int page_no = translate_table.frame[i].page_no;
            src = (char*) ((page_no < code_top)? APPS_ENTRY : APPS_ARG);
            dst = first_page + PAGE_SIZE * i;
            memcpy(dst, src + (page_no % code_top) * PAGE_SIZE, PAGE_SIZE);
            //cache_write(i, src + (page_no % code_top) * PAGE_SIZE);
        }

    /* map pid to virtual address space */
    for (int i = 0; i < NFRAMES; i++)
        if (FRAME_INUSE(i) && translate_table.frame[i].pid == pid) {
            src = first_page + i * PAGE_SIZE;
            int page_no = translate_table.frame[i].page_no;
            dst = (char*) ((page_no < code_top)? APPS_ENTRY : APPS_ARG);
            memcpy(dst + (page_no % code_top) * PAGE_SIZE, src, PAGE_SIZE);
        }

    curr_vm_pid = pid;
    return 0;
}


unsigned int frame_no, root;
void setup_identity_region(unsigned int region_addr, int npages) {
    int pde_flag = 0x1 | (1 << 6);  /* valid, point to next level */
    int pte_flag = 0xF | (1 << 6);  /* valid, readable, writable, executable */
    unsigned int leaf, mask = 0x3FFFFFFF;
    qemu_alloc(&frame_no, &leaf);
    memset((void*)leaf, 0, PAGE_SIZE);

    int vpn0 = (region_addr >> 12) & 0x3FF;  /* 10-bit */
    int vpn1 = (region_addr >> 22) & 0x3FF;  /* 10-bit */
    ((int*)root)[vpn1] = ((leaf >> 2) & mask) | pde_flag;
    for (int i = 0; i < npages; i++) {
        ((int*)leaf)[vpn0 + i] = (((region_addr + i * PAGE_SIZE) >> 2) & mask) | pte_flag;
    }

}

void create_identity_mapping() {
    /* Allocate the root page table and set the page table base */
    qemu_alloc(&frame_no, &root);
    memset((void*)root, 0, PAGE_SIZE);
    asm("csrw satp, %0" ::"r"(((root >> 12) & 0xFFFFF) | (1 << 31)));

    /* Allow all access to ITIM memory: 0800_0000 -> 0800_8000 */
    setup_identity_region(0x08000000, 8);    /* ITIM memory */
    setup_identity_region(0x80000000, 1024); /* DTIM memory */
    setup_identity_region(0x20400000, 1024); /* boot ROM */
    setup_identity_region(0x10013000, 1);    /* UART0 */
    setup_identity_region(0x02000000, 16);   /* UART0 */
    setup_identity_region(0x20800000, 1024); /* UART0 */
}

/* Implementation of MMU Initialization
 *
 * Detect whether egos-2000 is running on QEMU or the Arty board.
 * Choose the memory translation mechanism accordingly.
 */

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

    if (earth->platform == ARTY) {
        earth->tty_info("Use software translation for Arty");
        earth->mmu_free = arty_free;
        earth->mmu_alloc = arty_alloc;
        earth->mmu_map = arty_map;
        earth->mmu_switch = arty_switch;
        INFO("Will enter the grass layer with machine mode");
    } else {
        earth->tty_info("Use page table translation for QEMU");
        earth->mmu_free = qemu_free;
        earth->mmu_alloc = qemu_alloc;
        earth->mmu_map = qemu_map;
        earth->mmu_switch = qemu_switch;

        create_identity_mapping();
        INFO("Will enter the grass layer with supervisor mode");
        asm("csrr %0, mstatus" : "=r"(mstatus));
        asm("csrw mstatus, %0" ::"r"((mstatus & ~(3 << 11)) | (1 << 11) ));
    }

    curr_vm_pid = -1;
    memset(lookup_table, 0xFF, sizeof(lookup_table));
}
