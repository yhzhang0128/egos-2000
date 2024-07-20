#pragma once

struct elf32_header {
    uchar  e_ident[16];
    ushort e_type;
    ushort e_machine;
    uint   e_version;
    uint   e_entry;
    uint   e_phoff;
    uint   e_shoff;
    uint   e_flags;
    ushort e_ehsize;
    ushort e_phentsize;
    ushort e_phnum;
    ushort e_shentsize;
    ushort e_shnum;
    ushort e_shstrndx;
};

struct elf32_program_header {
    uint p_type;
    uint p_offset;
    uint p_vaddr;
    uint p_paddr;
    uint p_filesz;
    uint p_memsz;
    uint p_flags;
    uint p_align;
};

typedef void (*elf_reader)(uint block_no, char* dst);
void elf_load(int pid, elf_reader reader, int argc, void** argv);
