#pragma once

#define PAGE_SIZE       4096

#define MAX_NPAGES      12
#define MAX_NFRAMES     100
#define CACHED_NFRAMES  20

#define VADDR_START     0x80000000
#define VADDR_SIZE      VADDR_START + PAGE_SIZE * MAX_NPAGES

#define CACHE_START     0x8000C000
#define CACHE_SIZE      CACHE_START + PAGE_SIZE * CACHED_NFRAMES

struct frame {
    char content[PAGE_SIZE];
};

struct mapping {
    int pid;
    int page_no;
    int flag;
};
extern struct mapping mappings[MAX_NFRAMES];
