#pragma once

#include <sys/types.h> /* Define uint and ushort */
typedef unsigned long long ulonglong;

struct earth {
    /* CPU interface */
    int (*timer_reset)();
    int (*kernel_entry_init)(void (*entry)(uint, uint));

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

    /* Some information about earth layer configuration */
    enum { PAGE_TABLE, SOFT_TLB } translation;
    enum { ARTY, QEMU_SIFIVE, QEMU_LATEST } platform;
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
    int  (*sys_wait)();
    int  (*sys_send)(int pid, char* msg, uint size);
    int  (*sys_recv)(int* pid, char* buf, uint size);
};

extern struct earth *earth;
extern struct grass *grass;

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
#define GRASS_SIZE        0x00002800
#define GRASS_ENTRY       0x08002800  /* 8KB    grass code+data       */
                                       /* 12KB   earth data            */
                                       /* earth code is in QSPI flash  */


#ifndef LIBC_STDIO
/* Only earth/dev_tty.c uses LIBC_STDIO and does not need these macros */
#define printf   earth->tty_printf
#define INFO     earth->tty_info
#define FATAL    earth->tty_fatal
#define SUCCESS  earth->tty_success
#define CRITICAL earth->tty_critical
#endif

/* Platform specific configuration */
#define MSIP       (earth->platform == ARTY? 0x2000000UL : 0x2000004UL)
#define SPI_BASE   (earth->platform == ARTY? 0x10024000UL : 0x10050000UL)
#define UART0_BASE (earth->platform == QEMU_LATEST? 0x10010000UL : 0x10013000UL)

/* Memory-mapped I/O register access macros */
#define ACCESS(x) (*(__typeof__(*x) volatile *)(x))
#define REGW(base, offset) (ACCESS((unsigned int*)(base + offset)))
#define REGB(base, offset) (ACCESS((unsigned char*)(base + offset)))
