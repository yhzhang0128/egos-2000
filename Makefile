# (C) 2025, Cornell University
# All rights reserved.

# BOARD can be arty_a7_35t, arty_a7_100t, arty_s7_50, or tangnano20k
BOARD       = tangnano20k
QEMU        = qemu-system-riscv32

ifeq ($(TOOLCHAIN), GNU)
# Use the official GNU toolchain.
# https://github.com/riscv-collab/riscv-gnu-toolchain
RISCV_CC    = riscv32-unknown-elf-gcc
OBJDUMP     = riscv32-unknown-elf-objdump
OBJCOPY     = riscv32-unknown-elf-objcopy
else
# Use the pre-compiled GNU toolchain binaries from xPack.
# https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases
RISCV_CC    = riscv-none-elf-gcc
OBJDUMP     = riscv-none-elf-objdump
OBJCOPY     = riscv-none-elf-objcopy
endif

DEBUG       = build/debug
RELEASE     = build/release

APPS_DEPS   = apps/*.* library/egos.h library/*/* Makefile
EGOS_DEPS   = earth/* grass/* library/egos.h library/*/* Makefile

FILESYS     = 1
LDFLAGS     = -nostdlib -lc -lgcc
INCLUDE     = -Ilibrary -Ilibrary/elf -Ilibrary/file -Ilibrary/libc -Ilibrary/syscall
CFLAGS      = -march=rv32ima_zicsr -mabi=ilp32 -Wl,--gc-sections -ffunction-sections -fdata-sections -fdiagnostics-show-option
DEBUG_FLAGS = --source --all-headers --demangle --line-numbers --wide

SYSAPP_ELFS = $(patsubst %.c, $(RELEASE)/%.elf, $(notdir $(wildcard apps/system/*.c)))
USRAPP_ELFS = $(patsubst %.c, $(RELEASE)/user/%.elf, $(notdir $(wildcard apps/user/*.c)))

egos: $(USRAPP_ELFS) $(SYSAPP_ELFS) $(RELEASE)/egos.elf

$(RELEASE)/egos.elf: $(EGOS_DEPS)
	@printf "$(YELLOW)-------- Compile EGOS --------$(END)\n"
	$(RISCV_CC) $(CFLAGS) $(INCLUDE) -DKERNEL $(filter %.s, $(wildcard $^)) $(filter %.c, $(wildcard $^)) -Tlibrary/elf/egos.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(DEBUG)/egos.lst

$(SYSAPP_ELFS): $(RELEASE)/%.elf : apps/system/%.c $(APPS_DEPS)
	@printf "Compile app $(CYAN)%s$(END) => %s\n" $(patsubst %.c, %, $(notdir $<)) $@
	@$(RISCV_CC) $(CFLAGS) $(INCLUDE) -DFILESYS=$(FILESYS) -DKERNEL -Iapps apps/app.s $(filter %.c, $(wildcard $^)) -Tlibrary/elf/app.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(patsubst %.c, $(DEBUG)/%.lst, $(notdir $<))

$(USRAPP_ELFS): $(RELEASE)/user/%.elf : apps/user/%.c $(APPS_DEPS)
	@mkdir -p $(DEBUG) $(RELEASE) $(RELEASE)/user
	@printf "Compile app $(CYAN)%s$(END) => %s\n" $(patsubst %.c, %, $(notdir $<)) $@
	@$(RISCV_CC) $(CFLAGS) $(INCLUDE) -Iapps apps/app.s $(filter %.c, $(wildcard $^)) -Tlibrary/elf/app.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(patsubst %.c, $(DEBUG)/%.lst, $(notdir $<))

install: egos
	@printf "$(GREEN)-------- Create the Disk & BootROM Image --------$(END)\n"
	$(OBJCOPY) -O binary $(RELEASE)/egos.elf tools/qemu/egos.bin
	$(CC) tools/mkfs.c library/file/file$(FILESYS).c -DMKFS -DFILESYS=$(FILESYS) -DCPU_BIN_FILE="\"fpga/$(BOARD).bin\"" $(INCLUDE) -o tools/mkfs
	cd tools; rm -f disk.img bootROM.bin; ./mkfs

qemu: install
	@printf "$(YELLOW)-------- Simulate on QEMU-RISCV --------$(END)\n"
	$(QEMU) -nographic -readconfig tools/qemu/config.toml

program: install
	@printf "$(YELLOW)-------- Program the $(BOARD) on-board ROM --------$(END)\n"
	openFPGALoader -b $(BOARD) -f tools/bootROM.bin

clean:
	rm -rf build tools/mkfs tools/mkrom tools/qemu/egos.bin tools/disk.img tools/bootROM.bin

GREEN = \033[1;32m
YELLOW = \033[1;33m
CYAN = \033[1;36m
END = \033[0m
