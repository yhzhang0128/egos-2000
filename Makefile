# (C) 2024, Cornell University
# All rights reserved.

# BOARD can be a7_35t or a7_100t
BOARD       = a7_35t
QEMU        = qemu-system-riscv32

# GNU toolchain binaries from SiFive
RISCV_CC    = riscv64-unknown-elf-gcc -march=rv32ima
OBJDUMP     = riscv64-unknown-elf-objdump
OBJCOPY     = riscv64-unknown-elf-objcopy

DEBUG       = build/debug
RELEASE     = build/release

APPS_DEPS   = apps/*.* library/egos.h library/*/* Makefile
EGOS_DEPS   = earth/* grass/* library/egos.h library/*/* Makefile

FILESYS     = 1
LDFLAGS     = -nostdlib -lc -lgcc
INCLUDE     = -Ilibrary -Ilibrary/elf -Ilibrary/file -Ilibrary/libc -Ilibrary/syscall
CFLAGS      = -mabi=ilp32 -Wl,--gc-sections -ffunction-sections -fdata-sections -fdiagnostics-show-option
DEBUG_FLAGS = --source --all-headers --demangle --line-numbers --wide

SYSAPP_ELFS = $(patsubst %.c, $(RELEASE)/%.elf, $(notdir $(wildcard apps/system/*.c)))
USRAPP_ELFS = $(patsubst %.c, $(RELEASE)/user/%.elf, $(notdir $(wildcard apps/user/*.c)))

egos: $(USRAPP_ELFS) $(SYSAPP_ELFS) $(RELEASE)/egos.elf

$(RELEASE)/egos.elf: $(EGOS_DEPS)
	@echo "$(YELLOW)-------- Compile EGOS --------$(END)"
	$(RISCV_CC) $(CFLAGS) $(INCLUDE) -DKERNEL $(filter %.s, $(wildcard $^)) $(filter %.c, $(wildcard $^)) -Tlibrary/elf/egos.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(DEBUG)/egos.lst

$(SYSAPP_ELFS): $(RELEASE)/%.elf : apps/system/%.c $(APPS_DEPS)
	@echo "Compile app$(CYAN)" $(patsubst %.c, %, $(notdir $<)) "$(END)=>" $@
	@$(RISCV_CC) $(CFLAGS) $(INCLUDE) -DFILESYS=$(FILESYS) -DKERNEL -Iapps apps/app.s $(filter %.c, $(wildcard $^)) -Tlibrary/elf/app.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(patsubst %.c, $(DEBUG)/%.lst, $(notdir $<))

$(USRAPP_ELFS): $(RELEASE)/user/%.elf : apps/user/%.c $(APPS_DEPS)
	@mkdir -p $(DEBUG) $(RELEASE) $(RELEASE)/user
	@echo "Compile app$(CYAN)" $(patsubst %.c, %, $(notdir $<)) "$(END)=>" $@
	@$(RISCV_CC) $(CFLAGS) $(INCLUDE) -Iapps apps/app.s $(filter %.c, $(wildcard $^)) -Tlibrary/elf/app.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(patsubst %.c, $(DEBUG)/%.lst, $(notdir $<))

install: egos
	@echo "$(GREEN)-------- Create the Disk Image --------$(END)"
	$(OBJCOPY) -O binary $(RELEASE)/egos.elf tools/qemu/egos.bin
	$(CC) tools/mkfs.c library/file/file$(FILESYS).c -DMKFS -DFILESYS=$(FILESYS) $(INCLUDE) -o tools/mkfs; cd tools; ./mkfs
	@echo "$(YELLOW)-------- Create the BootROM Image --------$(END)"
	$(CC) tools/mkrom.c -DCPU_BIN_FILE="\"fpga/vexriscv/vexriscv_cpu_$(BOARD).bin\"" -o tools/mkrom
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
