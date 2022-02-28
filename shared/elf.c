/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: load an ELF file into memory; only using the single 
 * program header and not using the multiple section headers
 */

#include <string.h>

#include "egos.h"
#include "mem.h"
#include "elf.h"
#include "log.h"
#include "print.h"

static void load_grass(struct block_store* bs,
                       struct earth* earth,
                       struct elf32_program_header* pheader);

static void load_app(int pid,
                     struct block_store* bs,
                     struct earth* earth,
                     struct elf32_program_header* pheader);


void elf_load(int pid, struct block_store* bs, struct earth* earth) {
    char buf[BLOCK_SIZE];
    bs->read(0, buf);
    struct elf32_header *header = (void*) buf;
    if (header->e_phnum != 1 ||
        header->e_phoff + header->e_phentsize > BLOCK_SIZE) {
        FATAL("Grass exec region of the disk seems to be corrupted");
    }
    
    struct elf32_program_header pheader;
    memcpy(&pheader, buf + header->e_phoff, sizeof(pheader));

    if (pheader.p_vaddr == GRASS_ENTRY) {
        load_grass(bs, earth, &pheader);
    } else if (pheader.p_vaddr == APPS_BASE) {
        load_app(pid, bs, earth, &pheader);
    } else {
        FATAL("ELF gives invalid starting vaddr: 0x%.8x", pheader.p_vaddr);
    }
}

static void load_grass(struct block_store* bs,
                       struct earth* earth,
                       struct elf32_program_header* pheader) {
    INFO("Grass kernel file size: 0x%.8x bytes", pheader->p_filesz);
    INFO("Grass kernel memory size: 0x%.8x bytes", pheader->p_memsz);

    if (pheader->p_offset % BLOCK_SIZE) {
        FATAL("TODO: program offset not aligned by %d", BLOCK_SIZE);
    }

    int block_offset = pheader->p_offset / BLOCK_SIZE;
    for (int size = 0; size < pheader->p_filesz; size += BLOCK_SIZE) {
        bs->read(block_offset++, (char*)GRASS_ENTRY + size);
    }

    /* the last 0x80 bytes are reserved for struct earth */
    memset((char*)GRASS_ENTRY + pheader->p_filesz, 0, GRASS_SIZE - pheader->p_filesz - 0x80);
}

static void load_app(int pid,
                     struct block_store* bs,
                     struct earth* earth,
                     struct elf32_program_header* pheader) {
    INFO("App file size: 0x%.8x bytes", pheader->p_filesz);
    INFO("App memory size: 0x%.8x bytes", pheader->p_memsz);

    if (pheader->p_offset % BLOCK_SIZE) {
        FATAL("TODO: program offset not aligned by %d", BLOCK_SIZE);
    }

    /* load the application */
    int base, size, frame_no, page_no = 0;
    int block_offset = pheader->p_offset / BLOCK_SIZE;
    for (size = 0; size < pheader->p_filesz; size += BLOCK_SIZE) {
        if (size % PAGE_SIZE == 0) {
            earth->mmu_alloc(&frame_no, &base);
            earth->mmu_map(pid, page_no++, frame_no, F_ALL);
        }
        bs->read(block_offset++, ((char*)base) + (size % PAGE_SIZE));
    }
    int last_page_filled = pheader->p_filesz % PAGE_SIZE;
    int last_page_nzeros = PAGE_SIZE - last_page_filled;
    if (last_page_filled)
        memset(((char*)base) + last_page_filled, 0, last_page_nzeros);

    /* one more page for the heap */
    earth->mmu_alloc(&frame_no, &base);
    earth->mmu_map(pid, page_no++, frame_no, F_ALL);
    memset((char*)base, 0, PAGE_SIZE);

    /* two pages for the stack */
    earth->mmu_alloc(&frame_no, &base);
    earth->mmu_map(pid, MAX_NPAGES - 2, frame_no, F_ALL);

    earth->mmu_alloc(&frame_no, &base);
    earth->mmu_map(pid, MAX_NPAGES - 1, frame_no, F_ALL);    
}


