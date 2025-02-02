/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: ELF-format executable file loader
 * Only uses the program header instead of the section headers.
 */

#include "egos.h"
#include "elf.h"
#include "disk.h"
#include "servers.h"
#include <string.h>

void elf_load(int pid, elf_reader reader, int argc, void** argv) {
    /* Load the ELF header */
    char hbuf[BLOCK_SIZE], buf[BLOCK_SIZE];
    reader(0, hbuf);
    struct elf32_header* header          = (void*)hbuf;
    struct elf32_program_header* pheader = (void*)(hbuf + header->e_phoff);

    /* Load the code and data from the ELF-format binary executable file */
    for (uint i = 0; i < header->e_phnum; i++) {
        uint addr = pheader[i].p_vaddr;
        if (addr < RAM_START) continue;

        uint memsz        = pheader[i].p_memsz;
        uint filesz       = pheader[i].p_filesz;
        uint curr_pageno  = addr / PAGE_SIZE;
        uint end_pageno   = (addr + memsz) / PAGE_SIZE;
        uint curr_blockno = pheader[i].p_offset / BLOCK_SIZE;
        for (uint ppage_id, off = 0; off < filesz; off += BLOCK_SIZE) {
            /* Allocate one page (4KB) for every 8 blocks (512 bytes) */
            if (off % PAGE_SIZE == 0) {
                ppage_id = earth->mmu_alloc();
                earth->mmu_map(pid, curr_pageno++, ppage_id);
                memset(PAGE_ID_TO_ADDR(ppage_id), 0, PAGE_SIZE);
            }
            uint size =
                (off + BLOCK_SIZE < filesz) ? BLOCK_SIZE : (filesz - off);
            reader(curr_blockno++, buf);
            memcpy(PAGE_ID_TO_ADDR(ppage_id) + (off % PAGE_SIZE), buf, size);
        }

        while (curr_pageno < end_pageno) {
            uint ppage_id = earth->mmu_alloc();
            earth->mmu_map(pid, curr_pageno++, ppage_id);
            memset(PAGE_ID_TO_ADDR(ppage_id), 0, PAGE_SIZE);
        }

        /* Debug printing for kernel processes */
        if (pid < GPID_USER_START)
            INFO("Load 0x%x bytes to 0x%x", filesz, addr);
    }

    /* Setup two pages for main() args (argc/argv) and system call args */
    uint args_start = APPS_ARG / PAGE_SIZE;
    uint ppage_id   = earth->mmu_alloc();
    earth->mmu_map(pid, args_start++, ppage_id);

    int* argc_addr = (int*)PAGE_ID_TO_ADDR(ppage_id);
    int* argv_addr = argc_addr + 1;
    int* args_addr = argv_addr + CMD_NARGS;

    *argc_addr = argc;
    if (argv) memcpy(args_addr, argv, argc * CMD_ARG_LEN);
    for (uint i = 0; i < argc; i++)
        argv_addr[i] = APPS_ARG + sizeof(uint) /* argc */ +
                       sizeof(void*) * CMD_NARGS /* argv */ + i * CMD_ARG_LEN;

    ppage_id = earth->mmu_alloc();
    earth->mmu_map(pid, args_start++, ppage_id);

    /* Setup 2 pages for user stack (should be enough for demo purpose) */
#define APPS_STACK_NPAGES 2
#define APPS_STACK_SIZE   APPS_STACK_NPAGES* PAGE_SIZE
    uint stack_start = (APPS_STACK_TOP - APPS_STACK_SIZE) / PAGE_SIZE;
    for (uint i = 0; i < APPS_STACK_NPAGES; i++) {
        ppage_id = earth->mmu_alloc();
        earth->mmu_map(pid, stack_start++, ppage_id);
    }
}
