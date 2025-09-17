/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: wrapping the CPU interface for memory management unit (MMU)
 * This file contains functions for memory allocation/free, virtual memory
 * address translation (software TLB and page table), and cache flushing.
 */

#include "egos.h"
#include <string.h>

#define PAGE_SIZE          4096
#define PAGE_NO_TO_ADDR(x) (char*)(x * PAGE_SIZE)
#define PAGE_ID_TO_ADDR(x) ((char*)APPS_PAGES_BASE + x * PAGE_SIZE)
#define APPS_PAGES_CNT     (RAM_END - APPS_PAGES_BASE) / PAGE_SIZE

struct page_info {
    int use;
    int pid;
    uint vpage_no;
} page_info_table[APPS_PAGES_CNT];

uint mmu_alloc() {
    for (uint i = 0; i < APPS_PAGES_CNT; i++)
        if (!page_info_table[i].use) {
            page_info_table[i].use = 1;
            return i;
        }
    FATAL("mmu_alloc: no more free memory");
}

void mmu_free(int pid) {
    for (uint i = 0; i < APPS_PAGES_CNT; i++)
        if (page_info_table[i].use && page_info_table[i].pid == pid)
            memset(&page_info_table[i], 0, sizeof(struct page_info));
}

void soft_tlb_map(int pid, uint vpage_no, uint ppage_id) {
    page_info_table[ppage_id].pid      = pid;
    page_info_table[ppage_id].vpage_no = vpage_no;
}

void soft_tlb_switch(int pid) {
    static int curr_vm_pid = -1;
    if (pid == curr_vm_pid) return;

    /* Unmap curr_vm_pid from the user address space. */
    for (uint i = 0; i < APPS_PAGES_CNT; i++)
        if (page_info_table[i].use && page_info_table[i].pid == curr_vm_pid)
            memcpy(PAGE_ID_TO_ADDR(i),
                   PAGE_NO_TO_ADDR(page_info_table[i].vpage_no), PAGE_SIZE);

    /* Map pid to the user address space. */
    for (uint i = 0; i < APPS_PAGES_CNT; i++)
        if (page_info_table[i].use && page_info_table[i].pid == pid)
            memcpy(PAGE_NO_TO_ADDR(page_info_table[i].vpage_no),
                   PAGE_ID_TO_ADDR(i), PAGE_SIZE);

    curr_vm_pid = pid;
}

uint soft_tlb_translate(int pid, uint vaddr) {
    soft_tlb_switch(pid);
    return vaddr;
}

/* The code below creates an identity map using page tables (RISC-V Sv32). */
#define USER_RWX     (0xC0 | 0x1F)
#define MAX_NPROCESS 256
static uint* root;
static uint* leaf;
static uint* pid_to_pagetable_base[MAX_NPROCESS];
/* Assume at most MAX_NPROCESS unique processes just for simplicity. */

void setup_identity_region(int pid, uint addr, uint npages, uint flag) {
    uint vpn1 = addr >> 22;

    if (root[vpn1] & 0x1) {
        /* Leaf has been allocated. */
        leaf = (void*)((root[vpn1] << 2) & 0xFFFFF000);
    } else {
        /* Allocate the leaf page table. */
        uint ppage_id                 = earth->mmu_alloc();
        leaf                          = (void*)PAGE_ID_TO_ADDR(ppage_id);
        page_info_table[ppage_id].pid = pid;
        memset(leaf, 0, PAGE_SIZE);
        root[vpn1] = ((uint)leaf >> 2) | 0x1;
    }

    /* Setup the entries in the leaf page table. */
    uint vpn0 = (addr >> 12) & 0x3FF;
    for (uint i = 0; i < npages; i++)
        leaf[vpn0 + i] = ((addr + i * PAGE_SIZE) >> 2) | flag;
}

void pagetable_identity_map(int pid) {
    /* Allocate the root page table. */
    uint ppage_id                 = earth->mmu_alloc();
    root                          = (void*)PAGE_ID_TO_ADDR(ppage_id);
    page_info_table[ppage_id].pid = pid;
    pid_to_pagetable_base[pid]    = root;
    memset(root, 0, PAGE_SIZE);

    /* Setup the identity map for various memory regions. */
    for (uint i = RAM_START; i < RAM_END; i += PAGE_SIZE * 1024)
        setup_identity_region(pid, i, 1024, USER_RWX);    /* RAM   */
    setup_identity_region(pid, CLINT_BASE, 16, USER_RWX); /* CLINT */
    setup_identity_region(pid, UART_BASE, 1, USER_RWX);   /* UART  */
    setup_identity_region(pid, SPI_BASE, 1, USER_RWX);    /* SPI   */

    if (earth->platform == HARDWARE) {
        setup_identity_region(pid, BOARD_FLASH_ROM, 1024, USER_RWX);
        setup_identity_region(pid, ETHMAC_CSR_BASE, 1, USER_RWX);
        setup_identity_region(pid, ETHMAC_TX_BUFFER, 1, USER_RWX);
        setup_identity_region(pid, ETHMAC_RX_BUFFER, 1, USER_RWX);
    }
}

void page_table_map(int pid, uint vpage_no, uint ppage_id) {
    if (pid >= MAX_NPROCESS) FATAL("page_table_map: pid too large");

    /* Student's code goes here (Virtual Memory). */

    /* Remove the soft_tlb_map below and do the following.
     * (1) If page tables for pid do not exist, build the tables.
     *   Case#1: pid < GPID_USER_START
     * | Start Address | # Pages | Size   | Explanation                        |
     * +---------------+---------+--------+------------------------------------+
     * | 0x80000000    | 512     | 2 MB   | EGOS region (code+data+heap+stack) |
     * | 0x80200000    | 512     | 2 MB   | Apps region (code+data+heap+stack) |
     * | 0x80400000    | 1024    | 4 MB   | Free memory and video framebuffer  |
     * | CLINT_BASE    | 16      | 64 KB  | Memory-mapped registers for timer  |
     * | UART_BASE     | 1       | 4 KB   | Memory-mapped registers for TTY    |
     * | SPI_BASE      | 1       | 4 KB   | Memory-mapped registers for SD     |
     *
     *   Case#2: pid >= GPID_USER_START
     * | Start Address | # Pages | Size   | Explanation                        |
     * +---------------+---------+--------+------------------------------------+
     * | 0x80302000    | 1       | 4 KB   | Work dir (see apps/app.h)          |
     *
     * (2) After building page tables for pid (or if page tables for pid exist),
     *     update the page tables and map vpage_no to ppage_id based on Sv32. */
    soft_tlb_map(pid, vpage_no, ppage_id);

    /* Student's code ends here. */
}

void page_table_switch(int pid) {
    /* Student's code goes here (Virtual Memory). */

    /* Remove the soft_tlb_switch below and, instead, update the page table
     * base register (satp) using the value of pid_to_pagetable_base[pid].
     * An example of updating the satp CSR is given in function mmu_init. */
    soft_tlb_switch(pid);

    /* Student's code ends here. */
}

uint page_table_translate(int pid, uint vaddr) {
    /* Student's code goes here (Virtual Memory). */

    /* Remove the following line of code. Walk through the page tables
     * for process pid and return the physical address mapped from vaddr. */
    return soft_tlb_translate(pid, vaddr);

    /* Student's code ends here. */
}

void flush_cache() {
    if (earth->platform == HARDWARE) {
        /* Flush the L1 instruction cache. */
        /* See
         * https://github.com/yhzhang0128/litex/blob/egos/litex/soc/cores/cpu/vexriscv_smp/system.h#L9-L25
         */
        asm(".word(0x100F)\nnop\nnop\nnop\nnop\nnop\n");
    }
    if (earth->translation == PAGE_TABLE) {
        /* Flush the TLB, the cache for page table entries. */
        /* See
         * https://riscv.org/wp-content/uploads/2017/05/riscv-privileged-v1.10.pdf#subsection.4.2.1
         */
        asm("sfence.vma zero,zero");
    }
}

void mmu_init() {
    earth->mmu_free        = mmu_free;
    earth->mmu_alloc       = mmu_alloc;
    earth->mmu_flush_cache = flush_cache;

    /* Setup a PMP region for the whole 4GB address space. */
    asm("csrw pmpaddr0, %0" : : "r"(0x40000000));
    asm("csrw pmpcfg0, %0" : : "r"(0xF));

    /* Student's code goes here (System Call & Protection). */

    /* Replace the PMP region above with a NAPOT region 0x80200000 - 0x80400000
     * and set the permission for user mode access as r/w/x. */

    /* Student's code ends here. */

    CRITICAL("Choose a memory translation mechanism:");
    printf("Enter 0: page tables\n\rEnter 1: software TLB\n\r");

    char buf[2];
    for (buf[0] = 0; buf[0] != '0' && buf[0] != '1'; earth->tty_read(&buf[0]));
    earth->translation = (buf[0] == '0') ? PAGE_TABLE : SOFT_TLB;
    INFO("%s translation is chosen",
         earth->translation == PAGE_TABLE ? "Page table" : "Software");

    if (earth->translation == PAGE_TABLE) {
        /* Setup an identity map using page tables. */
        pagetable_identity_map(0);
        asm("csrw satp, %0" ::"r"(((uint)root >> 12) | (1 << 31)));

        earth->mmu_map       = page_table_map;
        earth->mmu_switch    = page_table_switch;
        earth->mmu_translate = page_table_translate;
    } else {
        earth->mmu_map       = soft_tlb_map;
        earth->mmu_switch    = soft_tlb_switch;
        earth->mmu_translate = soft_tlb_translate;
    }
}
