#pragma once

#include "egos.h"
#include "elf.h"
#include "disk.h"
#include <string.h>

void tty_init(struct earth*);
void disk_init(struct earth*);
void intr_init(struct earth*);
void mmu_init(struct earth*);
