# BOARD can be a7_35t, a7_100t or s7_50
BOARD = a7_35t
QEMU = qemu-system-riscv32

ifeq ($(TOOLCHAIN), GNU)
# The official GNU toolchain binaries
RISCV_CC = riscv32-unknown-elf-gcc -march=rv32im_zicsr
OBJDUMP = riscv32-unknown-elf-objdump
OBJCOPY = riscv32-unknown-elf-objcopy
else
# GNU toolchain binaries from SiFive
RISCV_CC = riscv64-unknown-elf-gcc -march=rv32i
OBJDUMP = riscv64-unknown-elf-objdump
OBJCOPY = riscv64-unknown-elf-objcopy
endif

DEBUG = build/debug
RELEASE = build/release

APPS_DEPS = apps/*.* library/egos.h library/*/*
GRASS_DEPS = grass/* library/egos.h library/*/*
EARTH_DEPS = earth/* earth/sd/* library/egos.h library/*/*
USRAPP_ELFS = $(patsubst %.c, $(RELEASE)/%.elf, $(notdir $(wildcard apps/user/*.c)))
SYSAPP_ELFS = $(patsubst %.c, $(RELEASE)/%.elf, $(notdir $(wildcard apps/system/*.c)))

LDFLAGS = -nostdlib -lc -lgcc
INCLUDE = -Ilibrary -Ilibrary/elf -Ilibrary/file -Ilibrary/libc -Ilibrary/servers
CFLAGS = -mabi=ilp32 -Wl,--gc-sections -ffunction-sections -fdata-sections -fdiagnostics-show-option
COMMON = $(CFLAGS) $(INCLUDE) -D CPU_CLOCK_RATE=65000000
DEBUG_FLAGS =  --source --all-headers --demangle --line-numbers --wide

egos: $(USRAPP_ELFS) $(SYSAPP_ELFS) $(RELEASE)/grass.elf $(RELEASE)/earth.elf

$(RELEASE)/earth.elf: $(EARTH_DEPS)
	@echo "$(YELLOW)-------- Compile the Earth Layer --------$(END)"
	$(RISCV_CC) $(COMMON) earth/earth.s $(filter %.c, $(wildcard $^)) -Tearth/earth.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(DEBUG)/earth.lst

$(RELEASE)/grass.elf: $(GRASS_DEPS)
	@echo "$(GREEN)-------- Compile the Grass Layer --------$(END)"
	$(RISCV_CC) $(COMMON) grass/grass.s $(filter %.c, $(wildcard $^)) -Tgrass/grass.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(DEBUG)/grass.lst

$(SYSAPP_ELFS): $(RELEASE)/%.elf : apps/system/%.c $(APPS_DEPS)
	@echo "Compile app$(CYAN)" $(patsubst %.c, %, $(notdir $<)) "$(END)=>" $@
	@$(RISCV_CC) $(COMMON) -Iapps apps/app.s $(filter %.c, $(wildcard $^)) -Tapps/app.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(patsubst %.c, $(DEBUG)/%.lst, $(notdir $<))

$(USRAPP_ELFS): $(RELEASE)/%.elf : apps/user/%.c $(APPS_DEPS)
	@mkdir -p $(DEBUG) $(RELEASE)
	@echo "Compile app$(CYAN)" $(patsubst %.c, %, $(notdir $<)) "$(END)=>" $@
	@$(RISCV_CC) $(COMMON) -Iapps apps/app.s $(filter %.c, $(wildcard $^)) -Tapps/app.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(patsubst %.c, $(DEBUG)/%.lst, $(notdir $<))

install: egos
	@echo "$(GREEN)-------- Create the Disk Image --------$(END)"
	$(CC) tools/mkfs.c library/file/file.c -DMKFS $(INCLUDE) -o tools/mkfs; cd tools; ./mkfs
	@echo "$(YELLOW)-------- Create the BootROM Image --------$(END)"
	cp $(RELEASE)/earth.elf tools/earth.elf
	$(OBJCOPY) --remove-section=.image tools/earth.elf
	$(OBJCOPY) -O binary tools/earth.elf tools/earth.bin
	$(CC) tools/mkrom.c -DCPU_BIN_FILE="\"fpga/freedom/fe310_cpu_$(BOARD).bin\"" -o tools/mkrom
	cd tools; ./mkrom ; rm earth.elf earth.bin

qemu: install
	@echo "$(YELLOW)-------- Simulate on QEMU-RISCV --------$(END)"
	cp $(RELEASE)/earth.elf tools/qemu/qemu.elf
	$(OBJCOPY) --update-section .image=tools/disk.img tools/qemu/qemu.elf
	$(QEMU) -readconfig tools/qemu/sifive-e31.cfg -kernel tools/qemu/qemu.elf -nographic

program: install
	@echo "$(YELLOW)-------- Program the Arty $(BOARD) on-board ROM --------$(END)"
	cd tools/fpga/openocd; time openocd -f 7series_$(BOARD).txt

clean:
	rm -rf build tools/mkfs tools/mkrom tools/qemu/qemu.elf tools/disk.img tools/bootROM.bin

GREEN = \033[1;32m
YELLOW = \033[1;33m
CYAN = \033[1;36m
END = \033[0m
