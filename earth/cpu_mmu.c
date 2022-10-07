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

enum {
      QEMU_PAGE_TABLE,
      ARTY_SOFTWARE_TLB
};

/* Implementation of Software TLB Translation
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


/* Implementation of Page Table Translation
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

/* Implementation of MMU Initialization
 *
 * Detect whether egos-2000 is running on QEMU or the Arty board.
 * Choose the memory translation mechanism accordingly.
 */

static int machine, tmp;
void machine_detect(int id) {
    machine = ARTY_SOFTWARE_TLB;
    /* Skip the illegal store instruction */
    asm("csrr %0, mepc" : "=r"(tmp));
    asm("csrw mepc, %0" ::"r"(tmp + 4));
}

int qemu_intr_enable() {
    int sstatus, sie;
    asm("csrr %0, sstatus" : "=r"(sstatus));
    asm("csrw sstatus, %0" ::"r"(sstatus | 0x2));
    asm("csrr %0, sie" : "=r"(sie));
    asm("csrw sie, %0" ::"r"(sie | 0x22));

    return 0;
}

void enter_supervisor_mode() {
    int ra, mstatus, mie;
    /* Set the program counter for mret */
    asm("mv %0, ra" :"=r"(ra));
    asm("csrw mepc, %0" ::"r"(ra));

    /* Set supervisor mode for mret and enable machine mode interrupts */
    asm("csrr %0, mstatus" : "=r"(mstatus));
    mstatus &= ~(3 << 11);
    mstatus |= (1 << 11);  /* supervisor mode is mode #1 */
    mstatus |= 0x8;        /* enable machine mode interrupt */
    asm("csrw mstatus, %0" ::"r"(mstatus));

    /* Delegate all exceptions/interrupts to the supervisor mode */
    asm("csrw medeleg, %0" :: "r" (0xffff));
    asm("csrw mideleg, %0" :: "r" (0xffff));
    earth->intr_enable = qemu_intr_enable;
    asm("csrw stvec, %0" ::"r"(supervisor_trap_entry));
    /* Enable machine mode software and timer interrupts */
    asm("csrr %0, mie" : "=r"(mie));
    asm("csrw mie, %0" ::"r"(mie | 0x88));

    /* Setup a PMP region allowing all memory access */
    __asm__ volatile("csrw pmpaddr0, %0" :: "r" (0x3fffffffffffffull));
    __asm__ volatile("csrw pmpcfg0, %0" :: "r" (0xf));
    /* Disable page table translation for now */
    asm("csrw satp, %0" :: "r" (0));

    INFO("Entering the supervisor mode with program counter %x", ra);
    asm("mret");
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

        /* Page table translation requires the supervisor mode */
        enter_supervisor_mode();
        earth->tty_success("Entered supervisor mode");
    }

    curr_vm_pid = -1;
    memset(lookup_table, 0xFF, sizeof(lookup_table));
}
