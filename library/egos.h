#pragma once

typedef unsigned char      uchar;
typedef unsigned short     ushort;
typedef unsigned int       uint;
typedef unsigned long long ulonglong;

struct earth {
    /* CPU interface */
    void (*timer_reset)(uint core_id);
    void (*mmu_flush_cache)();
    void (*mmu_alloc)(uint* ppage_id, void** ppage_addr);
    void (*mmu_free)(int pid);
    void (*mmu_map)(int pid, uint vpage_no, uint ppage_id);
    void (*mmu_switch)(int pid);

    /* Devices interface */
    void (*tty_read)(char* c);
    void (*tty_write)(char c);
    void (*disk_read)(uint block_no, uint nblocks, char* dst);
    void (*disk_write)(uint block_no, uint nblocks, char* src);

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
    void (*sys_send)(int receiver, char* msg, uint size);
    void (*sys_recv)(int from, int* sender, char* buf, uint size);
};

extern struct earth* earth;
extern struct grass* grass;

#define NCORES             4
#define release(x)         __sync_lock_release(&x)
#define acquire(x)         while(__sync_lock_test_and_set(&x, 1) != 0);

/* Memory regions */
#define PAGE_SIZE          4096
#define RAM_END            0x90000000 /* 256MB memory in total     */
#define APPS_PAGES_BASE    0x80800000 /* 248MB initially free      */

#define APPS_STACK_TOP     0x80800000 /* 2MB app stack             */
#define SYSCALL_ARG        0x80601000 /* struct syscall            */
#define APPS_ARG           0x80600000 /* main() argc/argv          */
#define APPS_ENTRY         0x80400000 /* 2MB app code and data     */

#define EGOS_STACK_TOP     0x80400000 /* 2MB egos stack            */
#define GRASS_STRUCT_BASE  0x80201000 /* struct grass              */
#define EARTH_STRUCT_BASE  0x80200000 /* struct earth              */
#define RAM_START          0x80000000 /* 2MB egos code and data    */

#define BOARD_FLASH_ROM    0x20400000 /* 4MB disk image on ROM, only on the Arty board */

/* Memory-mapped I/O regions */
#define ETHMAC_CSR_BASE    0xF0002000
#define ETHMAC_RX_BUFFER   0x90000000
#define ETHMAC_TX_BUFFER   0x90001000
#define SPI_BASE           (earth->platform == ARTY? 0xF0008800UL : 0x10050000UL)
#define UART_BASE          (earth->platform == ARTY? 0xF0001000UL : 0x10010000UL)
#define CLINT_BASE         (earth->platform == ARTY? 0xF0010000UL : 0x02000000UL)

/* Memory-mapped I/O register access macros */
#define ACCESS(x)          (*(__typeof__(*x) volatile *)(x))
#define REGW(base, offset) (ACCESS((uint*)(base + offset)))
#define REGB(base, offset) (ACCESS((uchar*)(base + offset)))

/* Printing functionalities defined in library/libc/print.c */
#define printf my_printf
int my_printf(const char* format, ...);

int INFO(const char* format, ...);
int FATAL(const char* format, ...);
int SUCCESS(const char* format, ...);
int CRITICAL(const char* format, ...);
