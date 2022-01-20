#pragma once

#define PAGE_SIZE       4096
#define MAX_NFRAMES     100
#define CACHED_NFRAMES  20
#define CACHE_START     0x8000C000

#define VADDR_START     0x80000000
#define MAX_NPAGES      12

struct frame_cache {
    char content[PAGE_SIZE];
};

struct mapping {
    int pid;
    int page_no;
    int flag;
};
extern struct mapping mappings[MAX_NFRAMES];
