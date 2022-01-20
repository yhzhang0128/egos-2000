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
#include "fs.h"
#include "elf.h"
#include "mmu.h"
#include "log.h"
#include "grass.h"

void elf_load(int pid, struct block_store* bs, struct earth* earth) {
    char buf[512];
    bs->read(0, 1, buf);
    struct elf32_header *header = (void*) buf;
    if (header->e_phnum != 1) {
        FATAL("Grass exec region of the disk seems to be corrupted");
    }
    
    if (header->e_phoff + header->e_phentsize > BLOCK_SIZE)  {
        FATAL("TODO: program header not in the first block of ELF");
    }
    
    struct elf32_program_header pheader;
    memcpy(&pheader, buf + header->e_phoff, sizeof(pheader));

    /* load the grass kernel */
    if (pheader.p_vaddr == GRASS_BASE) {
        INFO("Grass kernel starts at vaddr: 0x%.8x", pheader.p_vaddr);
        INFO("Grass kernel memory size: 0x%.8x bytes", pheader.p_memsz);

        if (pheader.p_offset % BLOCK_SIZE) {
            FATAL("TODO: program offset not aligned by %d", BLOCK_SIZE);
        }

        int block_offset = pheader.p_offset / BLOCK_SIZE;
        for (int size = 0; size < pheader.p_filesz; size += BLOCK_SIZE) {
            bs->read(block_offset++, 1, (char*)GRASS_BASE + size);
        }
        memset((char*)GRASS_BASE + pheader.p_filesz, 0, GRASS_SIZE - pheader.p_filesz);
        return;
    }

    /* load an application */
    if (pheader.p_vaddr == APPS_BASE) {
        INFO("App starts at vaddr: 0x%.8x", pheader.p_vaddr);
        INFO("App memory size: 0x%.8x bytes", pheader.p_memsz);

        if (pheader.p_offset % BLOCK_SIZE) {
            FATAL("TODO: program offset not aligned by %d", BLOCK_SIZE);
        }

        /* load the application */
        int base, frame_no, page_no = 0;
        int block_offset = pheader.p_offset / BLOCK_SIZE;
        for (int size = 0; size < pheader.p_filesz; size += BLOCK_SIZE) {
            if (size % PAGE_SIZE == 0) {
                earth->mmu_alloc(&frame_no, &base);
                earth->mmu_map(pid, page_no++, frame_no, F_ALL);
            }
            bs->read(block_offset++, 1, ((char*)base) + (size % PAGE_SIZE));
        }

        /* one more page for the heap */
        earth->mmu_alloc(&frame_no, &base);
        earth->mmu_map(pid, page_no++, frame_no, F_ALL);

        /* two more pages for the stack */
        earth->mmu_alloc(&frame_no, &base);
        earth->mmu_map(pid, MAX_NPAGES - 2, frame_no, F_ALL);

        earth->mmu_alloc(&frame_no, &base);
        earth->mmu_map(pid, MAX_NPAGES - 1, frame_no, F_ALL);
        return;
    }
    
    FATAL("ELF gives invalid starting vaddr: 0x%.8x", pheader.p_vaddr);
}

