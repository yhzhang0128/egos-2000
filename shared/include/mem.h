#pragma once

#define PAGE_SIZE           4096
#define NFRAMES             256
#define CACHED_NFRAMES      28

#define DTIM_END            0x80010000
#define FRAME_CACHE_START   0x80004000  // 112KB  frame cache
#define EARTH_STRUCT        0x80003f80  // 128B   earth interface
#define GRASS_SYSCALL_ARG   0x80003800  // 2KB    syscall arg
#define GRASS_STACK_TOP     0x80003800  // 6KB    kernel stack
#define APPS_STACK_TOP      0x80002000  // 8KB    app stack
#define DTIM_START          0x80000000

#define ITIM_END            0x08008000
#define APPS_SIZE           0x00002000
#define APPS_ENTRY          0x08006000  // 8KB    app code+data
#define GRASS_SIZE          0x00002000
#define GRASS_ENTRY         0x08004000  // 8KB    grass code+data
#define ITIM_START          0x08000000  // 16KB   earth data

/* F stands for memory frame */
#define F_INUSE             0x1
#define F_READ              0x2
#define F_WRITE             0x4
#define F_EXEC              0x8
#define F_ALL               0xf

#define INUSE(x) (x.flag & F_INUSE)
#define USE(x)   x.flag |= F_INUSE

