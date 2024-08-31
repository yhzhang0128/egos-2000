# (C) 2024, Cornell University
# All rights reserved.

QEMU        = qemu-system-riscv32
RISCV_CC    = riscv64-unknown-elf-gcc -march=rv32ima
OBJDUMP     = riscv64-unknown-elf-objdump
OBJCOPY     = riscv64-unknown-elf-objcopy

LDFLAGS     = -nostdlib -lc -lgcc
CFLAGS      = -mabi=ilp32 -Wl,--gc-sections -ffunction-sections -fdata-sections -fdiagnostics-show-option
DEBUG_FLAGS = --source --all-headers --demangle --line-numbers --wide

all:
	@echo "$(YELLOW)-------- Compile Hello, World! --------$(END)"
	$(RISCV_CC) $(CFLAGS) hello.s hello.c -Thello.lds $(LDFLAGS) -o hello.elf
	$(OBJDUMP) $(DEBUG_FLAGS) hello.elf > hello.lst
	$(OBJCOPY) -O binary hello.elf hello.bin

qemu: all
	@echo "$(YELLOW)-------- Run Hello-World on QEMU --------$(END)"
	$(QEMU) -nographic -machine sifive_u -smp 2 -bios hello.bin

clean:
	rm hello.bin hello.lst hello.elf

GREEN = \033[1;32m
YELLOW = \033[1;33m
CYAN = \033[1;36m
END = \033[0m
