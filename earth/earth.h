#pragma once

#include "egos.h"
#include "elf.h"
#include "disk.h"
#include "memory.h"
#include <string.h>

void tty_init();
void disk_init();
void intr_init();
void mmu_init();

typedef void (*handler_t)(int);
int intr_enable();
int intr_register(handler_t handler);
int excp_register(handler_t handler);

int mmu_alloc(int* frame_no, int* cached_addr);
int mmu_map(int pid, int page_no, int frame_no);
int mmu_switch(int pid);
int mmu_free(int pid);

int disk_read(int block_no, int nblocks, char* dst);
int disk_write(int block_no, int nblocks, char* src);

int tty_intr();
int tty_read(char* buf, int len);
int tty_printf(const char *format, ...);

int tty_info(const char *format, ...);
int tty_fatal(const char *format, ...);
int tty_success(const char *format, ...);
int tty_highlight(const char *format, ...);
