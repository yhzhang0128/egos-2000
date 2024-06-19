#pragma once

#include <sys/types.h> /* Define uint and ushort */
typedef unsigned long long ulonglong;

struct earth {
    int egos_lock, booted_core_cnt;

    /* CPU interface */
    int (*timer_reset)(uint core_id);
    int (*mmu_alloc)(uint* frame_no, void** cached_addr);
    int (*mmu_free)(int pid);
    int (*mmu_map)(int pid, uint page_no, uint frame_no);
    int (*mmu_switch)(int pid);

    /* Devices interface */
    int (*disk_read)(uint block_no, uint nblocks, char* dst);
    int (*disk_write)(uint block_no, uint nblocks, char* src);

    int (*tty_recv_intr)();
    int (*tty_read)(char* buf, uint len);
    int (*tty_write)(char* buf, uint len);

    int (*tty_printf)(const char *format, ...);
    int (*tty_info)(const char *format, ...);
    int (*tty_fatal)(const char *format, ...);
    int (*tty_success)(const char *format, ...);
    int (*tty_critical)(const char *format, ...);

    /* Earth configuration */
    enum { ARTY, QEMU } platform;
    enum { PAGE_TABLE, SOFT_TLB } translation;
};

struct grass {
    /* Shell environment variables */
    int workdir_ino;
    char workdir[128];

    /* Process control interface */
    int  (*proc_alloc)();
    void (*proc_free)(int pid);
    void (*proc_set_ready)(int pid);

    /* System call interface */
    void (*sys_exit)(int status);
    void (*sys_send)(int receiver, char* msg, uint size);
    void (*sys_recv)(int from, int* sender, char* buf, uint size);
};

extern struct earth *earth;
extern struct grass *grass;

#define NCORES            4
#define acquire(x)        while(__sync_lock_test_and_set(&x, 1) != 0);
#define release(x)        __sync_lock_release(&x)

/* Memory layout */
#define PAGE_SIZE         4096
#define FRAME_CACHE_END   0x80020000
#define FRAME_CACHE_START 0x80004000  /* 112KB  frame cache           */
                                      /*        earth interface       */
#define GRASS_STACK_TOP   0x80003f80  /* 8KB    earth/grass stack     */
                                      /*        grass interface       */
#define APPS_STACK_TOP    0x80002000  /* 6KB    app stack             */
#define SYSCALL_ARG       0x80000400  /* 1KB    system call args      */
#define APPS_ARG          0x80000000  /* 1KB    app main() argc, argv */
#define APPS_SIZE         0x00003000
#define APPS_ENTRY        0x08005000  /* 12KB   app code+data         */
#define GRASS_SIZE        0x00003000
#define GRASS_ENTRY       0x08002000  /* 8KB    grass code+data       */
                                      /* 12KB   earth data            */
                                      /* earth code is in QSPI flash  */

/* Platform specific configuration */
#define SPI_BASE   (earth->platform == ARTY? 0x10024000UL : 0x10050000UL)
#define UART0_BASE (earth->platform == ARTY? 0x10013000UL : 0x10010000UL)

#ifndef LIBC_STDIO
/* Only earth/dev_tty.c uses LIBC_STDIO and does not need these macros */
#define printf   earth->tty_printf
#define INFO     earth->tty_info
#define FATAL    earth->tty_fatal
#define SUCCESS  earth->tty_success
#define CRITICAL earth->tty_critical
#endif

/* Memory-mapped I/O register access macros */
#define ACCESS(x) (*(__typeof__(*x) volatile *)(x))
#define REGW(base, offset) (ACCESS((unsigned int*)(base + offset)))
#define REGB(base, offset) (ACCESS((unsigned char*)(base + offset)))
