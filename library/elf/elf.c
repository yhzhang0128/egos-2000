/*
 * (C) 2024, Cornell University
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
    char buf[BLOCK_SIZE];
    reader(0, buf);

    struct elf32_header* header          = (void*)buf;
    struct elf32_program_header* pheader = (void*)(buf + header->e_phoff);

    for (uint i = 0; i < header->e_phnum; i++) {
        if (pheader[i].p_memsz == 0) continue;
        if (pheader[i].p_vaddr == APPS_ENTRY) {
            pheader = &pheader[i];
            break;
        }
        FATAL("elf_load: Invalid p_vaddr: 0x%x", pheader->p_vaddr);
    }

    if (pid < GPID_USER_START) {
        INFO("App file size: %d bytes", pheader->p_filesz);
        INFO("App memory size: %d bytes", pheader->p_memsz);
    }

    uint ppage_id;
    uint code_start   = APPS_ENTRY >> 12;
    uint block_offset = pheader->p_offset / BLOCK_SIZE;

    /* Setup pages for text, rodata, data and bss sections */
    for (uint off = 0; off < pheader->p_filesz; off += BLOCK_SIZE) {
        if (off % PAGE_SIZE == 0) {
            ppage_id = earth->mmu_alloc();
            earth->mmu_map(pid, code_start++, ppage_id);
        }
        reader(block_offset++, PAGE_ID_TO_ADDR(ppage_id) + (off % PAGE_SIZE));
    }
    uint last_page_filled = pheader->p_filesz % PAGE_SIZE;
    uint last_page_nzeros = PAGE_SIZE - last_page_filled;
    if (last_page_filled)
        memset(PAGE_ID_TO_ADDR(ppage_id) + last_page_filled, 0,
               last_page_nzeros);

    while (code_start < ((APPS_ENTRY + pheader->p_memsz) >> 12)) {
        ppage_id = earth->mmu_alloc();
        earth->mmu_map(pid, code_start++, ppage_id);
        memset(PAGE_ID_TO_ADDR(ppage_id), 0, PAGE_SIZE);
    }

    /* Setup two pages for main() args (argc/argv) and syscall args */
    uint args_start = APPS_ARG >> 12;
    ppage_id        = earth->mmu_alloc();
    earth->mmu_map(pid, args_start++, ppage_id);

    int* argc_addr = (int*)PAGE_ID_TO_ADDR(ppage_id);
    int* argv_addr = argc_addr + 1;
    int* args_addr = argv_addr + CMD_NARGS;

    *argc_addr = argc;
    if (argv) memcpy(args_addr, argv, argc * CMD_ARG_LEN);
    for (uint i = 0; i < argc; i++)
        argv_addr[i] = APPS_ARG + 4 + 4 * CMD_NARGS + i * CMD_ARG_LEN;

    ppage_id = earth->mmu_alloc();
    earth->mmu_map(pid, args_start++, ppage_id);

    /* Setup two pages for user stack (should be enough for demo purpose) */
    uint stack_start = (APPS_STACK_TOP - PAGE_SIZE * 2) >> 12;
    ppage_id         = earth->mmu_alloc();
    earth->mmu_map(pid, stack_start++, ppage_id);

    ppage_id = earth->mmu_alloc();
    earth->mmu_map(pid, stack_start++, ppage_id);
}
