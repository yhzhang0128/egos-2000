# (C) 2024, Cornell University
# All rights reserved.

# BOARD can be a7_35t, a7_100t or s7_50
BOARD = a7_35t
QEMU = qemu-system-riscv32

ifeq ($(TOOLCHAIN), GNU)
# The official GNU toolchain binaries
RISCV_CC = riscv32-unknown-elf-gcc -march=rv32ima_zicsr
OBJDUMP = riscv32-unknown-elf-objdump
OBJCOPY = riscv32-unknown-elf-objcopy
else
# GNU toolchain binaries from SiFive
RISCV_CC = riscv64-unknown-elf-gcc -march=rv32ima
OBJDUMP = riscv64-unknown-elf-objdump
OBJCOPY = riscv64-unknown-elf-objcopy
endif

DEBUG = build/debug
RELEASE = build/release

APPS_DEPS = apps/*.* library/egos.h library/*/* Makefile
EGOS_DEPS = earth/* grass/* library/egos.h library/*/* Makefile

LDFLAGS = -nostdlib -lc -lgcc
INCLUDE = -Ilibrary -Ilibrary/elf -Ilibrary/file -Ilibrary/libc -Ilibrary/servers
CFLAGS = -mabi=ilp32 -Wl,--gc-sections -ffunction-sections -fdata-sections -fdiagnostics-show-option
COMMON = $(CFLAGS) $(INCLUDE) -D CPU_CLOCK_RATE=65000000
DEBUG_FLAGS = --source --all-headers --demangle --line-numbers --wide

USRAPP_ELFS = $(patsubst %.c, $(RELEASE)/%.elf, $(notdir $(wildcard apps/user/*.c)))
SYSAPP_ELFS = $(patsubst %.c, $(RELEASE)/%.elf, $(notdir $(wildcard apps/system/*.c)))

egos: $(USRAPP_ELFS) $(SYSAPP_ELFS) $(RELEASE)/egos.elf

$(RELEASE)/egos.elf: $(EGOS_DEPS)
	@echo "$(YELLOW)-------- Compile EGOS --------$(END)"
	$(RISCV_CC) $(COMMON) earth/boot.s $(filter %.c, $(wildcard $^)) -Tlibrary/linker/egos.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(DEBUG)/egos.lst

$(SYSAPP_ELFS): $(RELEASE)/%.elf : apps/system/%.c $(APPS_DEPS)
	@echo "Compile app$(CYAN)" $(patsubst %.c, %, $(notdir $<)) "$(END)=>" $@
	@$(RISCV_CC) $(COMMON) -Iapps apps/app.s $(filter %.c, $(wildcard $^)) -Tlibrary/linker/app.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(patsubst %.c, $(DEBUG)/%.lst, $(notdir $<))

$(USRAPP_ELFS): $(RELEASE)/%.elf : apps/user/%.c $(APPS_DEPS)
	@mkdir -p $(DEBUG) $(RELEASE)
	@echo "Compile app$(CYAN)" $(patsubst %.c, %, $(notdir $<)) "$(END)=>" $@
	@$(RISCV_CC) $(COMMON) -Iapps apps/app.s $(filter %.c, $(wildcard $^)) -Tlibrary/linker/app.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(patsubst %.c, $(DEBUG)/%.lst, $(notdir $<))

install: egos
	@echo "$(GREEN)-------- Create the Disk Image --------$(END)"
	$(OBJCOPY) -O binary $(RELEASE)/egos.elf tools/qemu/egos.bin
	$(CC) tools/mkfs.c library/file/file.c -DMKFS $(INCLUDE) -o tools/mkfs; cd tools; ./mkfs
	@echo "$(YELLOW)-------- Create the BootROM Image --------$(END)"
	$(CC) tools/mkrom.c -DCPU_BIN_FILE="\"fpga/freedom/fe310_cpu_$(BOARD).bin\"" -o tools/mkrom
	cd tools; ./mkrom

qemu: install
	@echo "$(YELLOW)-------- Simulate on QEMU-RISCV --------$(END)"
	$(QEMU) -nographic -readconfig tools/qemu/config.toml

program: install
	@echo "$(YELLOW)-------- Program the Arty $(BOARD) on-board ROM --------$(END)"
	cd tools/fpga/openocd; time openocd -f 7series_$(BOARD).txt

clean:
	rm -rf build earth/kernel_entry.lds tools/mkfs tools/mkrom tools/qemu/egos.bin tools/disk.img tools/bootROM.bin

GREEN = \033[1;32m
YELLOW = \033[1;33m
CYAN = \033[1;36m
END = \033[0m
