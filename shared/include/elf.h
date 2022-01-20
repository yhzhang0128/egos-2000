#pragma once

#include "fs.h"

#define uint32_t unsigned int
#define uint16_t unsigned short int

#define GRASS_BASE 0x08004000
#define GRASS_SIZE 0x00004000

#define APP_BASE   0x80000000
#define APP_SIZE   0x0000c000

struct elf32_header {
  unsigned char    e_ident[16];
  uint16_t         e_type;
  uint16_t         e_machine;
  uint32_t         e_version;
  uint32_t         e_entry;
  uint32_t         e_phoff;
  uint32_t         e_shoff;
  uint32_t         e_flags;
  uint16_t         e_ehsize;
  uint16_t         e_phentsize;
  uint16_t         e_phnum;
  uint16_t         e_shentsize;
  uint16_t         e_shnum;
  uint16_t         e_shstrndx;
};

struct elf32_program_header{
  uint32_t      p_type;
  uint32_t      p_offset;
  uint32_t      p_vaddr;
  uint32_t      p_paddr;
  uint32_t      p_filesz;
  uint32_t      p_memsz;
  uint32_t      p_flags;
  uint32_t      p_align;
};

struct earth;
void elf_load(struct block_store* bs, struct earth* earth);
