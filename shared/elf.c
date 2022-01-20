/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: load an ELF file into memory; only using the single 
 * program header and not using the multiple section headers
 */

#include "egos.h"
#include "elf.h"
#include "log.h"
#include "grass.h"
#include <string.h>

void elf_load(struct block_store* bs, struct earth* earth) {
    char buf[512];
    bs->read(0, 1, buf);

    struct elf32_header *header = (void*) buf;
    INFO("ELF program header table entry count: %d", header->e_phnum);

    if (header->e_phnum != 1) {
        FATAL("Grass exec region of the disk seems to be corrupted");
    }
    
    if (header->e_phoff + header->e_phentsize > BLOCK_SIZE)  {
        FATAL("TODO: program header not in the first block of ELF");
    }
    
    struct elf32_program_header pheader;
    memcpy(&pheader, buf + header->e_phoff, sizeof(pheader));

    if (pheader.p_vaddr == GRASS_BASE) {
        INFO("Grass kernel starts at vaddr: 0x%.8x", pheader.p_vaddr);
        INFO("Grass kernel memory size: 0x%.8x bytes", pheader.p_memsz);

        if (pheader.p_offset % BLOCK_SIZE) {
            FATAL("TODO: program offset not aligned by %d", BLOCK_SIZE);
        }

        /* load the grass kernel */
        int block_offset = pheader.p_offset / BLOCK_SIZE;
        for (int size = 0; size < pheader.p_filesz; size += BLOCK_SIZE) {
            bs->read(block_offset++, 1, (char*)GRASS_BASE + size);
        }
        memset((char*)GRASS_BASE + pheader.p_filesz, 0, GRASS_SIZE - pheader.p_filesz);


        /* call the grass kernel entry and never return */
        void (*grass_entry)() = (void*)GRASS_BASE;
        grass_entry();
    } else if (pheader.p_vaddr == APPS_BASE) {
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
                INFO("Allocated physical frame %d with base 0x%.8x", frame_no, (uint32_t)base);
            }
            bs->read(block_offset++, 1, ((char*)base) + (size % PAGE_SIZE));
        }

        earth->mmu_alloc(&frame_no, &base);
        INFO("Allocated physical frame %d with base 0x%.8x", frame_no, (uint32_t)base);
        
    } else {
        FATAL("ELF gives invalid starting vaddr: 0x%.8x", pheader.p_vaddr);
    }
}

