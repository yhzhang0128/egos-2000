#pragma once

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long ulonglong;

struct earth {
    uint (*mmu_alloc)();
    void (*mmu_free)(int pid);
    void (*mmu_flush_cache)();
    void (*timer_reset)(uint core_id);

    void (*mmu_map)(int pid, uint vpage_no, uint ppage_id);
    uint (*mmu_translate)(int pid, uint vaddr);
    void (*mmu_switch)(int pid);

    void (*tty_read)(char* c);
    void (*tty_write)(char c);
    uint (*tty_input_empty)();
    void (*disk_read)(uint block_no, uint nblocks, char* dst);
    void (*disk_write)(uint block_no, uint nblocks, char* src);

    enum { HARDWARE, QEMU } platform;
    enum { PAGE_TABLE, SOFT_TLB } translation;
};

struct grass {
    int (*proc_alloc)();
    void (*proc_free)(int pid);
    void (*proc_set_ready)(int pid);

    void (*sys_send)(int receiver, char* msg, uint size);
    void (*sys_recv)(int from, int* sender, char* buf, uint size);
    /* Student's code goes here (System Call | Multicore & Locks). */

    /* Add interface functions for process sleep and multicore information. */

    /* Student's code ends here. */
};

extern struct earth* earth;
extern struct grass* grass;

/* Below is the physical memory layout in egos-2000. */
#define RAM_END           0x80600000 /* 6MB memory [0x80000000,0x80600000)  */
#define APPS_PAGES_BASE   0x80400000 /* 2MB free for mmu_alloc              */
#define APPS_STACK_TOP    0x80400000 /* 1MB app stack (growing down)        */
#define SHELL_WORK_DIR    0x80302000 /* current work directory for shell    */
#define SYSCALL_ARG       0x80301000 /* struct syscall                      */
#define APPS_ARG          0x80300000 /* main() arguments (argc and argv)    */
#define APPS_ENTRY        0x80200000 /* 1MB app code and data               */
#define EGOS_STACK_TOP    0x80200000 /* 1MB egos stack (growing down)       */
#define GRASS_STRUCT_BASE 0x80101000 /* struct grass                        */
#define EARTH_STRUCT_BASE 0x80100000 /* struct earth                        */
#define RAM_START         0x80000000 /* 1MB egos code and data              */
#define BOARD_FLASH_ROM   0x20400000 /* 4MB disk image on FPGA board ROM    */

/* Below is the memory-mapped I/O layout in egos-2000. */
#define VGA_HDMI_BASE    0x80600000 /* 2MB video framebuffer */
#define ETHMAC_CSR_BASE  0xF0002000
#define ETHMAC_RX_BUFFER 0x90000000
#define ETHMAC_TX_BUFFER 0x90001000
#define SPI_BASE         (earth->platform == QEMU ? 0x10050000UL : 0xF0008800UL)
#define UART_BASE        (earth->platform == QEMU ? 0x10010000UL : 0xF0001000UL)
#define CLINT_BASE       (earth->platform == QEMU ? 0x02000000UL : 0xF0010000UL)

/* Below are some common macros/declarations for I/O, multicore and printing. */
#define ACCESS(x)          (*(__typeof__(*x) volatile*)(x))
#define REGW(base, offset) (ACCESS((uint*)(base + offset)))
#define REGB(base, offset) (ACCESS((uchar*)(base + offset)))

#define NCORES     4
#define release(x) __sync_lock_release(&x);
#define acquire(x) while (__sync_lock_test_and_set(&x, 1) != 0);
extern int boot_lock, kernel_lock, booted_core_cnt;

#define printf my_printf
int INFO(const char* format, ...);
int FATAL(const char* format, ...);
int SUCCESS(const char* format, ...);
int CRITICAL(const char* format, ...);
int my_printf(const char* format, ...);
