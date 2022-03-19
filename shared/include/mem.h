#pragma once

#define PAGE_SIZE           4096
#define NFRAMES             256
#define CACHED_NFRAMES      28

/* memory layout */
#define DTIM_END            0x80010000
#define FRAME_CACHE_START   0x80004000  // 112KB  frame cache
#define EARTH_SIZE          0x00000080
#define EARTH_STRUCT        0x80003f80  //        earth interface
#define GRASS_STACK_TOP     0x80003f80  // ~8KB   kernel stack
#define GRASS_STRUCT        0x80002000  //        grass interface
#define APPS_STACK_TOP      0x80002000  // ~6KB   app stack
#define GRASS_SYSCALL_ARG   0x80000400  // ~1KB   syscall args
#define APPS_MAIN_ARG       0x80000000  // ~1KB   main() args
#define DTIM_START          0x80000000

#define ITIM_END            0x08008000
#define APPS_SIZE           0x00003000  // 12KB   app code+data
#define APPS_ENTRY          0x08005000
#define GRASS_SIZE          0x00002000  // 8KB    grass code+data
#define GRASS_ENTRY         0x08003000
#define ITIM_START          0x08000000  // 12KB   earth data+stack
                                        // earth code is in QSPI flash

/* F stands for memory frame */
#define F_INUSE             0x1
#define F_READ              0x2
#define F_WRITE             0x4
#define F_EXEC              0x8
#define F_ALL               0xf

#undef  malloc
#define malloc my_alloc
void* my_alloc(unsigned int size);

#undef  free
#define free my_free
void my_free(void* ptr);
