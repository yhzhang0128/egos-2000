/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: load an ELF-format executable file into memory
 * Only use the program header instead of the section headers.
 */

#include "egos.h"
#include "elf.h"
#include "disk.h"
#include "servers.h"

#include <string.h>

static void load_app(int pid, elf_reader reader,
                     int argc, void** argv,
                     struct elf32_program_header* pheader) {

    /* Debug printing during bootup */
    if (pid < GPID_USER_START) {
        INFO("App file size: 0x%.8x bytes", pheader->p_filesz);
        INFO("App memory size: 0x%.8x bytes", pheader->p_memsz);
    }

    void* base;
    uint ppage_id, block_offset = pheader->p_offset / BLOCK_SIZE;
    uint code_start = APPS_ENTRY >> 12;

    /* Setup the text, rodata, data and bss sections */
    for (uint off = 0; off < pheader->p_filesz; off += BLOCK_SIZE) {
        if (off % PAGE_SIZE == 0) {
            earth->mmu_alloc(&ppage_id, &base);
            earth->mmu_map(pid, code_start++, ppage_id);
        }
        reader(block_offset++, (char*)base + (off % PAGE_SIZE));
    }
    uint last_page_filled = pheader->p_filesz % PAGE_SIZE;
    uint last_page_nzeros = PAGE_SIZE - last_page_filled;
    if (last_page_filled)
        memset((char*)base + last_page_filled, 0, last_page_nzeros);

    while (code_start < ((APPS_ENTRY + pheader->p_memsz) >> 12)) {
        earth->mmu_alloc(&ppage_id, &base);
        earth->mmu_map(pid, code_start++, ppage_id);
        memset((char*)base, 0, PAGE_SIZE);
    }

    /* Setup two pages for main() args and syscall args */
    uint args_start = APPS_ARG >> 12;
    earth->mmu_alloc(&ppage_id, &base);
    earth->mmu_map(pid, args_start++, ppage_id);

    int* argc_addr = (int*)base;
    int* argv_addr = argc_addr + 1;
    int* args_addr = argv_addr + CMD_NARGS;

    *argc_addr = argc;
    if (argv) memcpy(args_addr, argv, argc * CMD_ARG_LEN);
    for (uint i = 0; i < argc; i++)
        argv_addr[i] = APPS_ARG + 4 + 4 * CMD_NARGS + i * CMD_ARG_LEN;

    earth->mmu_alloc(&ppage_id, &base);
    earth->mmu_map(pid, args_start++, ppage_id);

    /* Setup two pages for user stack (should be enough for demo purpose) */
    uint stack_start = (APPS_STACK_TOP - PAGE_SIZE * 2) >> 12;
    earth->mmu_alloc(&ppage_id, &base);
    earth->mmu_map(pid, stack_start++, ppage_id);

    earth->mmu_alloc(&ppage_id, &base);
    earth->mmu_map(pid, stack_start++, ppage_id);
}

void elf_load(int pid, elf_reader reader, int argc, void** argv) {
    char buf[BLOCK_SIZE];
    reader(0, buf);

    struct elf32_header *header = (void*) buf;
    struct elf32_program_header *pheader = (void*)(buf + header->e_phoff);

    for (uint i = 0; i < header->e_phnum; i++) {
        if (pheader[i].p_memsz == 0) continue;
        else if (pheader[i].p_vaddr == APPS_ENTRY)
            load_app(pid, reader, argc, argv, &pheader[i]);
        else FATAL("elf_load: Invalid p_vaddr: 0x%.8x", pheader->p_vaddr);
    }
}
