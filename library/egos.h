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
#define RAM_END         0x80600000UL /* 6MB memory starting at RAM_START */
#define APPS_PAGES_BASE 0x80400000UL /* 2MB free for mmu_alloc           */
#define APPS_STACK_TOP  0x80400000UL /* 1MB app stack (growing down)     */
#define SHELL_WORK_DIR  0x80302000UL /* current work directory for shell */
#define SYSCALL_ARG     0x80301000UL /* struct syscall                   */
#define APPS_ARG        0x80300000UL /* main() arguments (argc and argv) */
#define APPS_ENTRY      0x80200000UL /* 1MB app code and data            */
#define EGOS_STACK_TOP  0x80200000UL /* 1MB egos stack (growing down)    */
#define GRASS_STRUCT    0x80101000UL /* struct grass                     */
#define EARTH_STRUCT    0x80100000UL /* struct earth                     */
#define RAM_START       0x80000000UL /* 1MB egos code and data           */

/* Below is the memory-mapped I/O layout in egos-2000. */
#define SDHCI_PCI_ECAM   0x30008000UL /* QEMU     */
#define SDHCI_BASE       0x40000000UL /* QEMU     */
#define SDSPI_BASE       0xF0008000UL /* Hardware */
#define WIFI_BASE        0xF0003000UL /* Hardware */
#define PLIC_BASE        0x0C000000UL /* QEMU     */
#define ETH_PCI_ECAM     0x30018000UL /* QEMU     */
#define ETH_BUF_BASE     0x90000000UL /* Hardware */
#define ETH_CTL_BASE     (earth->platform == QEMU ? 0x41000000UL : 0xF0002000UL)
#define UART_BASE        (earth->platform == QEMU ? 0x10000000UL : 0xF0001000UL)
#define CLINT_BASE       (earth->platform == QEMU ? 0x02000000UL : 0xF0010000UL)
#define FLASH_ROM_BASE   (earth->platform == QEMU ? 0x22000000UL : 0x20400000UL)
#define VIDEO_FRAME_BASE (earth->platform == QEMU ? 0x41000000UL : 0x80600000UL)

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

/* Student's code goes here (Ethernet & TCP/IP). */

/* Define a data structure for Ethernet's RX descriptors. Declare the RX buffers
 * and the RX descriptors array with `extern` -- they are needed in earth/boot.c
 * for Ethernet controller initialization, and in grass/kernel.c for handling an
 * external interrupt upon receiving an Ethernet frame. */

/* Student's code ends here. */
