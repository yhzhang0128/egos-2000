/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: functions for memory management unit (MMU)
 * This MMU code contains a simple memory allocation/free mechanism
 * and two memory translation mechanisms: software TLB and page table.
 * It also contains a hardware-specific cache flushing function.
 */

#include "egos.h"
#include "disk.h"
#include "servers.h"
#include <string.h>

/* Memory allocation and free */
#define APPS_PAGES_CNT (RAM_END - APPS_PAGES_BASE) / PAGE_SIZE
struct page_info {
    int  use;      /* Is this page free or allocated? */
    int  pid;      /* Which process owns this page? */
    uint vpage_no; /* Which virtual page in this process maps to this physial page? */
} page_info_table[APPS_PAGES_CNT];

#define PAGE_NO_TO_ADDR(x) (char*)(x * PAGE_SIZE)
#define PAGE_ID_TO_ADDR(x) (char*)APPS_PAGES_BASE + x * PAGE_SIZE

void mmu_alloc(uint* ppage_id, void** ppage_addr) {
    for (uint i = 0; i < APPS_PAGES_CNT; i++)
        if (!page_info_table[i].use) {
            page_info_table[i].use = 1;
            *ppage_id              = i;
            *ppage_addr            = PAGE_ID_TO_ADDR(i);
            return;
        }
    FATAL("mmu_alloc: no more free memory");
}

void mmu_free(int pid) {
    for (uint i = 0; i < APPS_PAGES_CNT; i++)
        if (page_info_table[i].use && page_info_table[i].pid == pid)
            memset(&page_info_table[i], 0, sizeof(struct page_info));
}

/* Software TLB Translation */
void soft_tlb_map(int pid, uint vpage_no, uint ppage_id) {
    page_info_table[ppage_id].pid      = pid;
    page_info_table[ppage_id].vpage_no = vpage_no;
}

void soft_tlb_switch(int pid) {
    static int curr_vm_pid = -1;
    if (pid == curr_vm_pid) return;

    /* Unmap curr_vm_pid from the user address space */
    for (uint i = 0; i < APPS_PAGES_CNT; i++)
        if (page_info_table[i].use && page_info_table[i].pid == curr_vm_pid)
            memcpy(PAGE_ID_TO_ADDR(i),
                   PAGE_NO_TO_ADDR(page_info_table[i].vpage_no),
                   PAGE_SIZE);

    /* Map pid to the user address space */
    for (uint i = 0; i < APPS_PAGES_CNT; i++)
        if (page_info_table[i].use && page_info_table[i].pid == pid)
            memcpy(PAGE_NO_TO_ADDR(page_info_table[i].vpage_no),
                   PAGE_ID_TO_ADDR(i),
                   PAGE_SIZE);

    curr_vm_pid = pid;
}

/* Page Table Translation
 *
 * The code below creates an identity mapping using RISC-V Sv32;
 * Read section4.3 of RISC-V manual (references/riscv-privileged-v1.10.pdf)
 *
 * mmu_map() and mmu_switch() using page tables is given to students
 * as a course project. After this project, every process should have
 * its own set of page tables. mmu_map() will modify entries in these
 * tables and mmu_switch() will modify satp (page table base register)
 */

#define OS_RWX       (0xC0 | 0xF)
#define USER_RWX     (0xC0 | 0x1F)
#define MAX_NPROCESS 256
static uint* root;
static uint* leaf;
static uint* pid_to_pagetable_base[MAX_NPROCESS];
/* Assume at most MAX_NPROCESS unique processes for simplicity */

void setup_identity_region(int pid, uint addr, uint npages, uint flag) {
    uint vpn1 = addr >> 22;

    if (root[vpn1] & 0x1) {
        /* Leaf has been allocated */
        leaf = (void*)((root[vpn1] << 2) & 0xFFFFF000);
    } else {
        /* Leaf has not been allocated */
        uint ppage_id;
        earth->mmu_alloc(&ppage_id, (void**)&leaf);
        page_info_table[ppage_id].pid = pid;
        memset(leaf, 0, PAGE_SIZE);
        root[vpn1] = ((uint)leaf >> 2) | 0x1;
    }

    /* Setup the entries in the leaf page table */
    uint vpn0 = (addr >> 12) & 0x3FF;
    for (uint i = 0; i < npages; i++)
        leaf[vpn0 + i] = ((addr + i * PAGE_SIZE) >> 2) | flag;
}

void pagetable_identity_mapping(int pid) {
    /* Allocate the root page table and set the page table base (satp) */
    uint ppage_id;
    earth->mmu_alloc(&ppage_id, (void**)&root);
    memset(root, 0, PAGE_SIZE);
    page_info_table[ppage_id].pid = pid;
    pid_to_pagetable_base[pid]    = root;

    /* Allocate the leaf page tables */
    for (uint i = RAM_START; i < RAM_END; i += PAGE_SIZE * 1024)
        setup_identity_region(pid, i, 1024, OS_RWX);    /* RAM   */
    setup_identity_region(pid, CLINT_BASE, 16, OS_RWX); /* CLINT */
    setup_identity_region(pid, UART_BASE, 1,  OS_RWX);  /* UART  */
    setup_identity_region(pid, SPI_BASE,   1,  OS_RWX); /* SPI   */

    if (earth->platform == ARTY) {
        setup_identity_region( pid, BOARD_FLASH_ROM, 1024, OS_RWX); /* ROM */
        setup_identity_region( pid, ETHMAC_CSR_BASE,  1, OS_RWX);   /* ETHMAC CSR */
        setup_identity_region( pid, ETHMAC_TX_BUFFER, 1, OS_RWX);   /* ETHMAC TX buffer */
        setup_identity_region( pid, ETHMAC_RX_BUFFER, 1, OS_RWX);   /* ETHMAC RX buffer */
    } else {
        /* Student's code goes here (networking) */

        /* Create page tables for the GEM region of the sifive_u machine */
        /* Reference: https://github.com/qemu/qemu/blob/stable-9.0/hw/riscv/sifive_u.c#L86 */

        /* Student's code ends here. */
    }
}

void page_table_map(int pid, uint vpage_no, uint ppage_id) {
    if (pid >= MAX_NPROCESS) FATAL("page_table_map: pid too large");

    /* Student's code goes here (page table translation). */

    /* Remove the following line of code and, instead,
     * (1) if page tables for pid do not exist, build the tables;
     * (2) if page tables for pid exist, update entries of the tables
     *
     * Feel free to modify and call the two helper functions:
     * pagetable_identity_mapping() and setup_identity_region()
     */
    soft_tlb_map(pid, vpage_no, ppage_id);

    /* Student's code ends here. */
}

void page_table_switch(int pid) {
    /* Student's code goes here (page table translation). */

    /* Remove the following line of code and, instead,
     * modify the page table base register (satp) similar to
     * the code in mmu_init(); Remember to use the pid argument
     */
    soft_tlb_switch(pid);

    /* Student's code ends here. */
}

void flush_cache() {
    if (earth->platform == ARTY) {
        /* Flush the L1 instruction cache */
        /* See https://github.com/yhzhang0128/litex/blob/egos/litex/soc/cores/cpu/vexriscv_smp/system.h#L9-L25 */
        asm(".word(0x100F)\nnop\nnop\nnop\nnop\nnop\n");
    }
    if (earth->translation == PAGE_TABLE){
        /* Flush the TLB, which is the cache of page table entries */
        /* See https://riscv.org/wp-content/uploads/2017/05/riscv-privileged-v1.10.pdf#subsection.4.2.1 */
        asm("sfence.vma zero,zero");
    }
}

/* MMU Initialization */
void mmu_init() {
    earth->mmu_free = mmu_free;
    earth->mmu_alloc = mmu_alloc;
    earth->mmu_flush_cache = flush_cache;

    /* Setup a PMP region for the whole 4GB address space */
    asm("csrw pmpaddr0, %0" : : "r" (0x40000000));
    asm("csrw pmpcfg0, %0" : : "r" (0xF));

    /* Student's code goes here (PMP memory protection). */

    /* Setup PMP NAPOT region 0x80400000 - 0x80800000 as r/w/x */

    /* Student's code ends here. */

    /* Choose memory translation mechanism in QEMU */
    CRITICAL("Choose a memory translation mechanism:");
    printf("Enter 0: page tables\r\nEnter 1: software TLB\r\n");

    char buf[2];
    for (buf[0] = 0; buf[0] != '0' && buf[0] != '1'; earth->tty_read(&buf[0]));
    earth->translation = (buf[0] == '0') ? PAGE_TABLE : SOFT_TLB;
    INFO("%s translation is chosen", earth->translation == PAGE_TABLE ? "Page table" : "Software");

    if (earth->translation == PAGE_TABLE) {
        /* Setup an identity mapping using page tables */
        pagetable_identity_mapping(0);
        asm("csrw satp, %0" ::"r"(((uint)root >> 12) | (1 << 31)));

        earth->mmu_map    = page_table_map;
        earth->mmu_switch = page_table_switch;
    } else {
        earth->mmu_map    = soft_tlb_map;
        earth->mmu_switch = soft_tlb_switch;
    }
}
