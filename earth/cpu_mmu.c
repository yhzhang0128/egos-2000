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

void setup_identity_mapping() {
    /* In PMP, allow access to all physical memory */
    //asm("csrw pmpaddr0, %0" :: "r" (0x08008000));
    asm("csrw pmpaddr0, %0" :: "r" (0x3FFFFFFFFFFFFFULL));
    asm("csrw pmpcfg0, %0" :: "r" (0XF));

    /* int mstatus; */
    /* asm("csrr %0, mstatus" : "=r"(mstatus)); */
    /* asm("csrw mstatus, %0" ::"r"(mstatus | (3 << 18))); */
    
    int frame_no, vpn0, vpn1, root, leaf1, leaf2, leaf3, leaf4, leaf5, leaf6;
    int pde_flag = 0x1 | (1 << 6);  /* valid, point to next level */
    int pte_flag = 0xF | (1 << 6);  /* valid, readable, writable, executable */

    /* Allocate and empty the root page table */
    qemu_alloc(&frame_no, &root);
    memset((void*)root, 0, PAGE_SIZE);

    /* Set the page table base */
    int satp, root_ppn = (root >> 12) & 0xFFFFF;
    asm("csrw satp, %0" ::"r"(root_ppn | (1 << 31)));
    asm("csrr %0, satp" : "=r"(satp));
    INFO("Set satp as 0x%x", satp);

    /* Allow all access to ITIM memory: 0800_0000 -> 0800_8000 */
    unsigned int region1_addr = 0x08000000, region1_npages = 8;
    qemu_alloc(&frame_no, &leaf1);
    memset((void*)leaf1, 0, PAGE_SIZE);

    vpn0 = (region1_addr >> 12) & 0x3FF;  /* 10-bit */
    vpn1 = (region1_addr >> 22) & 0x3FF;  /* 10-bit */
    ((int*)root)[vpn1] = ((leaf1 >> 2) & 0x3FFFFFFF) | pde_flag;
    for (int i = 0; i < region1_npages; i++) {
        ((int*)leaf1)[vpn0 + i] = (((region1_addr + i * PAGE_SIZE) >> 2) & 0x3FFFFFFFF) | pte_flag;
    }

    /* Allow all access to DTIM memory: 8000_0000 -> 8040_0000 */
    unsigned int region2_addr = 0x80000000, region2_npages = 1024;
    qemu_alloc(&frame_no, &leaf2);
    memset((void*)leaf2, 0, PAGE_SIZE);

    vpn0 = (region2_addr >> 12) & 0x3FF;  /* 10-bit */
    vpn1 = (region2_addr >> 22) & 0x3FF;  /* 10-bit */
    ((int*)root)[vpn1] = ((leaf2 >> 2) & 0x3FFFFFFF) | pde_flag;
    for (int i = 0; i < region2_npages; i++) {
        ((int*)leaf2)[vpn0 + i] = (((region2_addr + i * PAGE_SIZE) >> 2) & 0x3FFFFFFFF) | pte_flag;
    }

    /* Allow all access to ROM: 2040_0000 -> 2080_0000 */
    unsigned int region3_addr = 0x20400000, region3_npages = 1024;
    qemu_alloc(&frame_no, &leaf3);
    memset((void*)leaf3, 0, PAGE_SIZE);

    vpn0 = (region3_addr >> 12) & 0x3FF;  /* 10-bit */
    vpn1 = (region3_addr >> 22) & 0x3FF;  /* 10-bit */
    ((int*)root)[vpn1] = ((leaf3 >> 2) & 0x3FFFFFFF) | pde_flag;
    for (int i = 0; i < region3_npages; i++) {
        ((int*)leaf3)[vpn0 + i] = (((region3_addr + i * PAGE_SIZE) >> 2) & 0x3FFFFFFFF) | pte_flag;
    }

    /* Allow all access to UART0: 1001_3000 -> 1014_0000 */
    unsigned int region4_addr = 0x10013000;
    qemu_alloc(&frame_no, &leaf4);
    memset((void*)leaf4, 0, PAGE_SIZE);

    vpn0 = (region4_addr >> 12) & 0x3FF;  /* 10-bit */
    vpn1 = (region4_addr >> 22) & 0x3FF;  /* 10-bit */
    ((int*)root)[vpn1] = ((leaf4 >> 2) & 0x3FFFFFFF) | pde_flag;
    ((int*)leaf4)[vpn0] = (((region4_addr) >> 2) & 0x3FFFFFFFF) | pte_flag;

    /* Allow all access to CLINT: 0200_0000 -> 0201_0000 */
    unsigned int region5_addr = 0x02000000, region5_npages = 16;
    qemu_alloc(&frame_no, &leaf5);
    memset((void*)leaf5, 0, PAGE_SIZE);

    vpn0 = (region5_addr >> 12) & 0x3FF;  /* 10-bit */
    vpn1 = (region5_addr >> 22) & 0x3FF;  /* 10-bit */
    ((int*)root)[vpn1] = ((leaf5 >> 2) & 0x3FFFFFFF) | pde_flag;
    for (int i = 0; i < region5_npages; i++) {
        ((int*)leaf5)[vpn0 + i] = (((region5_addr + i * PAGE_SIZE) >> 2) & 0x3FFFFFFFF) | pte_flag;
    }

    /* Allow all access to Disk: 2080_0000 -> 20c0_0000 */
    unsigned int region6_addr = 0x20800000, region6_npages = 1024;
    qemu_alloc(&frame_no, &leaf6);
    memset((void*)leaf6, 0, PAGE_SIZE);

    vpn0 = (region6_addr >> 12) & 0x3FF;  /* 10-bit */
    vpn1 = (region6_addr >> 22) & 0x3FF;  /* 10-bit */
    ((int*)root)[vpn1] = ((leaf6 >> 2) & 0x3FFFFFFF) | pde_flag;
    for (int i = 0; i < region6_npages; i++) {
        ((int*)leaf6)[vpn0 + i] = (((region6_addr + i * PAGE_SIZE) >> 2) & 0x3FFFFFFFF) | pte_flag;
    }
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

        setup_identity_mapping();
        INFO("Will enter the grass layer with supervisor mode");
        asm("csrr %0, mstatus" : "=r"(mstatus));
        asm("csrw mstatus, %0" ::"r"((mstatus & ~(3 << 11)) | (1 << 11) ));
    }

    curr_vm_pid = -1;
    memset(lookup_table, 0xFF, sizeof(lookup_table));
}
