#pragma once

#include <sys/types.h> /* Define uint and ushort */
typedef unsigned long long ulonglong;

struct earth {
    int boot_lock, kernel_lock, booted_core_cnt;

    /* CPU interface */
    int (*timer_reset)(uint core_id);
    int (*mmu_alloc)(uint* ppage_id, void** ppage_addr);
    int (*mmu_map)(int pid, uint vpage_no, uint ppage_id);
    int (*mmu_switch)(int pid);
    int (*mmu_free)(int pid);

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
    void (*proc_coresinfo)();
    void (*proc_set_ready)(int pid);
    void (*proc_set_idle)(uint core_id);

    /* System call interface */
    void (*sys_exit)(int status);
    void (*sys_send)(int receiver, char* msg, uint size);
    void (*sys_recv)(int from, int* sender, char* buf, uint size);
};

extern struct earth *earth;
extern struct grass *grass;

#define NCORES            4
#define release(x)        __sync_lock_release(&x)
#define acquire(x)        while(__sync_lock_test_and_set(&x, 1) != 0);

/* Memory regions */
#define PAGE_SIZE         4096
#define RAM_END           0x90000000 /* 256MB memory in total     */
#define APPS_PAGES_BASE   0x80800000 /* 248MB initially free      */

#define APPS_STACK_TOP    0x80800000 /* 2MB app stack             */
#define SYSCALL_ARG       0x80601000 /* struct syscall            */
#define APPS_ARG          0x80600000 /* main() argc/argv          */
#define APPS_ENTRY        0x80400000 /* 2MB app code and data     */

#define EGOS_STACK_TOP    0x80400000 /* 2MB egos stack            */
#define GRASS_STRUCT_BASE 0x80300800
#define EARTH_STRUCT_BASE 0x80300000 /* struct earth/grass        */
#define EGOS_HEAP_END     0x80200000 /* kernel message buffer     */
#define RAM_START         0x80000000 /* 2MB egos code and data    */

/* Memory-mapped I/O regions */
#define CLINT_BASE 0x02000000
#define SPI_BASE   (earth->platform == ARTY? 0x10024000UL : 0x10050000UL)
#define UART0_BASE (earth->platform == ARTY? 0x10013000UL : 0x10010000UL)

/* Memory-mapped I/O register access macros */
#define ACCESS(x) (*(__typeof__(*x) volatile *)(x))
#define REGW(base, offset) (ACCESS((unsigned int*)(base + offset)))
#define REGB(base, offset) (ACCESS((unsigned char*)(base + offset)))

/* Only earth/dev_tty.c uses LIBC_STDIO and does not need these macros */
#ifndef LIBC_STDIO
#define printf   earth->tty_printf
#define INFO     earth->tty_info
#define FATAL    earth->tty_fatal
#define SUCCESS  earth->tty_success
#define CRITICAL earth->tty_critical
#endif
