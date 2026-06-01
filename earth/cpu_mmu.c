/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: wrapping the CPU interface for memory management unit (MMU)
 * This file contains functions for memory allocation/free, virtual memory
 * address translation (software TLB and page table), and cache flushing.
 */

#include "egos.h"
#include "servers.h"
#include <string.h>

#define PAGE_SIZE          4096
#define PAGE_NO_TO_ADDR(x) (char*)(x * PAGE_SIZE)
#define PAGE_ID_TO_ADDR(x) ((char*)APPS_PAGES_BASE + x * PAGE_SIZE)
#define APPS_PAGES_CNT     (RAM_END - APPS_PAGES_BASE) / PAGE_SIZE
// 2MB data, is split into 512 pages; each page is 4kb

//assigned to a specific physical page
struct page_info {
    int use; // is the physical page currently in use or not
    int pid; // which process is this page associated with
    uint vpage_no; // virtual page number (i.e., virtual address / PAGE_SIZE) mapped to this page
    int is_page_table; // whether this physical page stores a page table
    // 0x803FF is virtual page number, XXX is offset
    // 0x80406 is physical page number
    // 0x803FFXXX --> 0x80406XXX
} page_info_table[APPS_PAGES_CNT];
// since page is 4kb = 2^12 bytes, so last 3 digits in hexadecimal is wtv; every other gets translated from virtual to physical

uint mmu_alloc() { //returns a physical page id, thats free to use
    for (uint i = 0; i < APPS_PAGES_CNT; i++)
        if (!page_info_table[i].use) {
            page_info_table[i].use = 1;
            return i;
        }
    FATAL("mmu_alloc: no more free memory");
}

void mmu_free(int pid) {
    uint released_pages = 0, page_table_pages = 0;

    for (uint i = 0; i < APPS_PAGES_CNT; i++) {
        if (page_info_table[i].use && page_info_table[i].pid == pid) {
            released_pages++;
            if (page_info_table[i].is_page_table) page_table_pages++;
            memset(&page_info_table[i], 0, sizeof(struct page_info));
        }
    }

    INFO("mmu_free released %d pages (%d are page tables) for process %d",
         released_pages, page_table_pages, pid);
}

void soft_tlb_map(int pid, uint vpage_no, uint ppage_id) { //ppage_id is phyiscal page id
    page_info_table[ppage_id].pid      = pid;
    page_info_table[ppage_id].vpage_no = vpage_no;
}
/*
we dont do a translation of memory addresses
- this version of TLB just copies data back and forth?
no hardware address translation at runtime
- we just copy data from physical page to virtual page, and vice versa, when we switch processes

this is current implementation:
- our goal is to implement Page Table Translation
*/
void soft_tlb_switch(int pid) {
    static int curr_vm_pid = -1; // which process's virtual memory is currently mapped to the user address space; -1 means no process is mapped (e.g., when we are running the kernel or idle)
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

// how to set up table translation, to translate identity
void setup_identity_region(int pid, uint addr, uint npages, uint flag) {
    uint vpn1 = addr >> 22;
    // root[vpn1] points to one leaf page, that can map a 4MB chunk of virtual memory
    // stores in root[vpn1] as (address >> 2) | flags
    if (root[vpn1] & 0x1) {
        /* Leaf has been allocated. */
        leaf = (void*)((root[vpn1] << 2) & 0xFFFFF000); // get the memory address, page aligned
    } else {
        /* Allocate the leaf page table. */
        uint ppage_id                 = earth->mmu_alloc();
        leaf                          = (void*)PAGE_ID_TO_ADDR(ppage_id); // convert physical page id to physical address
        page_info_table[ppage_id].pid = pid;
        page_info_table[ppage_id].is_page_table = 1;
        memset(leaf, 0, PAGE_SIZE); // clear the newly allocated leaf page table
        root[vpn1] = ((uint)leaf >> 2) | 0x1; // denote it as allocated, shift by 2 
    }

    /* Setup the entries in the leaf page table. */
    uint vpn0 = (addr >> 12) & 0x3FF; // get VPN0, clear out the last 12 bits, and get the last 10 bits
    // 0x3FF is 10 bits of 1s, so we get the last 10 bits of the virtual page number, which is the index into the leaf page table
    // maps each leaf entry to a physical page, with the given flag; each leaf entry is (physical address >> 2) | flag
    for (uint i = 0; i < npages; i++)
        leaf[vpn0 + i] = ((addr + i * PAGE_SIZE) >> 2) | flag;
}

void pagetable_identity_map(int pid) {
    /* Allocate the root page table. */
    uint ppage_id                 = earth->mmu_alloc();
    root                          = (void*)PAGE_ID_TO_ADDR(ppage_id);
    page_info_table[ppage_id].pid = pid;
    page_info_table[ppage_id].is_page_table = 1;
    pid_to_pagetable_base[pid]    = root; // for a given PID, where is the root of page table held
    memset(root, 0, PAGE_SIZE); //set all to 0

    /* Setup the identity map for various memory regions. */
    // PAGE_size = 4kb, so the loop does 4MB at a time, since each leaf page table can map 4MB of virtual memory
    for (uint i = RAM_START; i < RAM_END; i += PAGE_SIZE * 1024)
        setup_identity_region(pid, i, 1024, USER_RWX);

    //set up memory mapped IO regions
    setup_identity_region(pid, ETH_CTL_BASE, 4, USER_RWX);
    setup_identity_region(pid, UART_BASE, 1, USER_RWX);
    setup_identity_region(pid, CLINT_BASE, 16, USER_RWX);
    setup_identity_region(pid, FLASH_ROM_BASE, 1024, USER_RWX);
    setup_identity_region(pid, VIDEO_FRAME_BASE, 512, USER_RWX);

    if (earth->platform == QEMU) {
        setup_identity_region(pid, ETH_PCI_ECAM, 1, USER_RWX);
        setup_identity_region(pid, SDHCI_BASE, 1, USER_RWX);
    } else {
        setup_identity_region(pid, SDSPI_BASE, 1, USER_RWX);
        setup_identity_region(pid, WIFI_BASE, 1, USER_RWX);
        setup_identity_region(pid, ETH_BUF_BASE, 2, USER_RWX);
    }
}
/*
Page table translation
- two layer approach
- last 12 bits are offset (4kb), like before

so we have |VPN[1] | VPN[0] | offset|
- VPN[1] (10 bits) is index into root page table, which gives us the physical address of the leaf page table
- VPN[0] (10 bits) is index into leaf page table, which gives us the physical address of the page
- offset (12 bits) is the offset within the page

satp CSR register tells us the physical address of root page table
- note that this address is page-aligned

When a process accesses a virtual address:
- extract VPN[1] and VPN[0]
- use VPN[1] to index into root page table, get physical address of leaf page table
- use VPN[0] to index into leaf page table, get physical address of page
- use offset to get the final physical address

Translation Lookaside Buffer (TLB) is a cache for page table entries, to speed up the translation process
- without TLB, would have to walk the page tables every time we access a virtual address, which is slow
- with TLB, we can cache the recent translations, so that we can translate virtual addresses faster


*/


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
     * | 0x80400000    | 512     | 2 MB   | Initially free memory              |
     * | CLINT_BASE    | 16      | 64 KB  | Memory-mapped registers for timer  |
     * | UART_BASE     | 1       | 4 KB   | Memory-mapped registers for TTY    |
     * | SDHCI_BASE    | 1       | 4 KB   | Memory-mapped registers for SD     |
     *
     *   Case#2: pid >= GPID_USER_START
     * | Start Address | # Pages | Size   | Explanation                        |
     * +---------------+---------+--------+------------------------------------+
     * | 0x80302000    | 1       | 4 KB   | Work dir (see apps/app.h)          |
     * You may also map the regions for the Ethernet, WiFi and VGA/HDMI devices.
     *
     * (2) After building page tables for pid (or if page tables for pid exist),
     *     update the page tables and map vpage_no to ppage_id based on Sv32. */
    
    //  soft_tlb_map(pid, vpage_no, ppage_id); // soft TLB map does the copying of memory, replace it with identity map for now
    
    if (!pid_to_pagetable_base[pid]) {
        if (pid < GPID_USER_START) {
            pagetable_identity_map(pid);
        } else {
            uint root_ppage_id = earth->mmu_alloc();
            root = (void*)PAGE_ID_TO_ADDR(root_ppage_id);
            memset(root, 0, PAGE_SIZE);

            page_info_table[root_ppage_id].pid = pid;
            page_info_table[root_ppage_id].is_page_table = 1;
            pid_to_pagetable_base[pid] = root;
            setup_identity_region(pid, SHELL_WORK_DIR, 1, USER_RWX);
        }
    }
    uint* page_table_root = pid_to_pagetable_base[pid];
    root = page_table_root;
    if (!page_table_root) FATAL("page_table_map: no page table for pid %d", pid);
    uint vpn1 = vpage_no >> 10; // get the VPN[1] from the virtual page number
    uint vpn0 = vpage_no & 0x3FF; // get the VPN[0] from the virtual page number

    if (root[vpn1] & 0x1) {
        leaf = (void*)((root[vpn1] << 2) & 0xFFFFF000);
    } else {
        /* Allocate the leaf page table. */
        uint ppage_id                 = earth->mmu_alloc();
        leaf                          = (void*)PAGE_ID_TO_ADDR(ppage_id); // convert physical page id to physical address
        page_info_table[ppage_id].pid = pid;
        page_info_table[ppage_id].is_page_table = 1;
        memset(leaf, 0, PAGE_SIZE); // clear the newly allocated leaf page table
        root[vpn1] = ((uint)leaf >> 2) | 0x1; // denote it as allocated, shift by 2 
    }

    leaf[vpn0] = ((APPS_PAGES_BASE + ppage_id * PAGE_SIZE) >> 2) | USER_RWX; // map the virtual page to the physical page, with the given flag

    page_info_table[ppage_id].pid      = pid;
    page_info_table[ppage_id].vpage_no = vpage_no;
    /* Student's code ends here. */
}

void page_table_switch(int pid) {
    /* Student's code goes here (Virtual Memory). */

    /* Remove the soft_tlb_switch below and, instead, update the page table
     * base register (satp) using the value of pid_to_pagetable_base[pid].
     * An example of updating the satp CSR is given in function mmu_init. */
    // soft_tlb_switch(pid);

    if (pid >= MAX_NPROCESS) FATAL("page_table_switch: pid too large");
    uint* page_table_root = pid_to_pagetable_base[pid];
    if (!page_table_root) FATAL("page_table_switch: no page table for pid %d", pid);    
    asm("csrw satp, %0" ::"r"(((uint)page_table_root >> 12 | (1 << 31))));

    /* Student's code ends here. */
}

uint page_table_translate(int pid, uint vaddr) {
    /* Student's code goes here (Virtual Memory). */

    /* Remove the following line of code. Walk through the page tables
     * for process pid and return the physical address mapped from vaddr. */
    
    if (pid >= MAX_NPROCESS) FATAL("page_table_translate: pid too large");
    
    uint* page_table_root = pid_to_pagetable_base[pid];
    if (!page_table_root) FATAL("page_table_translate: no page table for pid %d", pid);
    uint vpn1 = vaddr >> 22;
    if(!(page_table_root[vpn1] & 0x1)) FATAL("No allocated root page found for pid %d and vaddr %x", pid, vaddr);
    uint* leaf = (void*)((page_table_root[vpn1] << 2) & 0xFFFFF000);
    uint vpn0 = (vaddr >> 12) & 0x3FF;
    if(!(leaf[vpn0] & 0x1)) FATAL("No allocated leaf page found for pid %d and vaddr 0x%x", pid, vaddr);
    uint paddr = ((leaf[vpn0] << 2) & (0xFFFFF000)) | (vaddr & 0xFFF);
    return paddr;

    // return soft_tlb_translate(pid, vaddr);

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

    CRITICAL("Choose a memory translation mechanism:");
    printf("Enter 0: page tables\n\rEnter 1: software TLB\n\r");

    char buf[2];
    for (buf[0] = 0; buf[0] != '0' && buf[0] != '1'; earth->tty_read(&buf[0]));
    earth->translation = (buf[0] == '0') ? PAGE_TABLE : SOFT_TLB;
    INFO("%s translation is chosen",
         earth->translation == PAGE_TABLE ? "Page table" : "Software");

    /* Student's code goes here (System Call & Protection). */

    /* Replace the PMP region above with a NAPOT region 0x80200000 - 0x80400000
     * and set the permission for user mode access as r/w/x. */
    if (earth->translation == SOFT_TLB) {
        asm("csrw pmpaddr0, %0" : : "r"(0x200BFFFF));
        asm("csrw pmpcfg0, %0" : : "r"(0x1F));
    }

    /* Student's code ends here. */

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
