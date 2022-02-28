#pragma once

#define PAGE_SIZE           4096
#define MAX_NPAGES          12
#define MAX_NFRAMES         256
#define CACHED_NFRAMES      20

#define VADDR_START         0x80000000
#define VADDR_SIZE          PAGE_SIZE * MAX_NPAGES

#define CACHE_START         0x8000C000
#define CACHE_SIZE          PAGE_SIZE * CACHED_NFRAMES

#define EARTH_STRUCT        0x80003f80 // 0x80004000 - 128

#define GRASS_ENTRY         0x08004000
#define GRASS_SIZE          0x00002000
#define GRASS_STACK_TOP     0x80003000

#define APPS_BASE           0x80000000
#define APPS_SIZE           0x0000c000

/* F stands for memory frame */
#define F_INUSE             0x1
#define F_READ              0x2
#define F_WRITE             0x4
#define F_EXEC              0x8
#define F_ALL               0xf

#define INUSE(x) (x.flag & F_INUSE)
#define USE(x)   x.flag |= F_INUSE

