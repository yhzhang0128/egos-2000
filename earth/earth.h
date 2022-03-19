#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "mem.h"
#include "elf.h"
#include "disk.h"
#include "servers.h"

int tty_init();
int tty_read(char* buf, int len);
int tty_write(const char *format, ...);

int disk_init();
int disk_busy();
int disk_read(int block_no, int nblocks, char* dst);
int disk_write(int block_no, int nblocks, char* src);

typedef void (*handler_t)(int);
int intr_init();
int intr_enable();
int intr_disable();
int intr_register(handler_t handler);
int excp_register(handler_t handler);

int mmu_init();
int mmu_alloc(int* frame_no, int* addr);
int mmu_map(int pid, int page_no, int frame_no, int flag);
int mmu_switch(int pid);
int mmu_free(int pid);

int log_info(const char *format, ...);
int log_highlight(const char *format, ...);
int log_success(const char *format, ...);
int log_fatal(const char *format, ...);

#define INFO       log_info
#define HIGHLIGHT  log_highlight
#define SUCCESS    log_success
#define FATAL      log_fatal