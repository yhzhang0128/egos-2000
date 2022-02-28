#pragma once

#define PAGE_SIZE           4096

#define MAX_NPAGES          12
#define MAX_NFRAMES         256
#define CACHED_NFRAMES      20

#define VADDR_START         0x80000000
#define VADDR_SIZE          PAGE_SIZE * MAX_NPAGES

#define CACHE_START         0x8000C000
#define CACHE_SIZE          PAGE_SIZE * CACHED_NFRAMES

#define F_INUSE             0x1
#define F_READ              0x2
#define F_WRITE             0x4
#define F_EXEC              0x8
#define F_ALL               0xf

struct frame_t {
    char byte[PAGE_SIZE];
};

struct translation_table_t {
    struct {
        int pid;
        int page_no;
        int flag;
    } frame[MAX_NFRAMES];
};

