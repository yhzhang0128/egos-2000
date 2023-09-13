# BOARD can be a7_35t, a7_100t or s7_50
BOARD = a7_35t
# TOOLCHAIN can be SIFIVE or GNU
TOOLCHAIN = SIFIVE

ifeq ($(TOOLCHAIN), SIFIVE)
# Toolchain binaries from SiFive
RISCV_CC = riscv64-unknown-elf-gcc -march=rv32i
OBJDUMP = riscv64-unknown-elf-objdump
OBJCOPY = riscv64-unknown-elf-objcopy
else
# Toolchain binaries from the official GNU
RISCV_CC = riscv32-unknown-elf-gcc -march=rv32im_zicsr
OBJDUMP = riscv32-unknown-elf-objdump
OBJCOPY = riscv32-unknown-elf-objcopy
endif
RISCV_QEMU = qemu-system-riscv32

DEBUG = build/debug
RELEASE = build/release

APPS_SRCS = apps/app.s grass/grass.s library/*/*.c
GRASS_SRCS = grass/grass.s grass/*.c library/elf/*.c
EARTH_SRCS = earth/earth.s earth/*.c earth/sd/*.c library/elf/*.c library/libc/*.c

LDFLAGS = -Wl,--gc-sections -nostartfiles -nostdlib
INCLUDE = -Ilibrary -Ilibrary/elf -Ilibrary/file -Ilibrary/libc -Ilibrary/servers
CFLAGS = -mabi=ilp32 -mcmodel=medlow -ffunction-sections -fdata-sections -fdiagnostics-show-option

COMMON = $(CFLAGS) $(LDFLAGS) $(INCLUDE) -D CPU_CLOCK_RATE=65000000
OBJDUMP_FLAGS = --source --all-headers --demangle --line-numbers --wide

all: apps
	@echo "$(GREEN)-------- Compile the Grass Layer --------$(END)"
	$(RISCV_CC) $(COMMON) $(GRASS_SRCS) -Tgrass/grass.lds -lc -lgcc -o $(RELEASE)/grass.elf
	$(OBJDUMP) $(OBJDUMP_FLAGS) $(RELEASE)/grass.elf > $(DEBUG)/grass.lst
	@echo "$(YELLOW)-------- Compile the Earth Layer --------$(END)"
	$(RISCV_CC) $(COMMON) $(EARTH_SRCS)  -Tearth/earth.lds -lc -lgcc -o $(RELEASE)/earth.elf
	$(OBJDUMP) $(OBJDUMP_FLAGS) $(RELEASE)/earth.elf > $(DEBUG)/earth.lst

.PHONY: apps
apps: apps/system/*.c apps/user/*.c
	mkdir -p $(DEBUG) $(RELEASE)
	@echo "$(CYAN)-------- Compile the Apps Layer --------$(END)"
	for FILE in $^ ; do \
	  export APP=$$(basename $${FILE} .c);\
	  echo "Compile" $${FILE} "=>" $(RELEASE)/$${APP}.elf;\
	  $(RISCV_CC) $(COMMON) $(APPS_SRCS) $${FILE} -Tapps/app.lds -lc -lgcc -Iapps -o $(RELEASE)/$${APP}.elf || exit 1 ;\
	  echo "Compile" $${FILE} "=>" $(DEBUG)/$${APP}.lst;\
	  $(OBJDUMP) $(OBJDUMP_FLAGS) $(RELEASE)/$${APP}.elf > $(DEBUG)/$${APP}.lst;\
	done

install:
	@echo "$(YELLOW)-------- Create the Disk Image --------$(END)"
	$(CC) tools/mkfs.c library/file/file.c -DMKFS $(INCLUDE) -o tools/mkfs; cd tools; ./mkfs
	@echo "$(YELLOW)-------- Create the BootROM Image --------$(END)"
	cp $(RELEASE)/earth.elf tools/earth.elf
	$(OBJCOPY) --remove-section=.image tools/earth.elf
	$(OBJCOPY) -O binary tools/earth.elf tools/earth.bin
	$(CC) tools/mkrom.c -DCPU_BIN_FILE="\"fpga/freedom/fe310_cpu_$(BOARD).bin\"" -o tools/mkrom
	cd tools; ./mkrom ; rm earth.elf earth.bin

program:
	@echo "$(YELLOW)-------- Program the on-board ROM --------$(END)"
	@echo "$(YELLOW)[WARNING]$(END) Make sure the board connected is Arty $(BOARD)."
	cd tools/fpga/openocd; time openocd -f 7series_$(BOARD).txt

qemu:
	@echo "$(YELLOW)-------- Simulate on QEMU-RISCV --------$(END)"
	cp $(RELEASE)/earth.elf tools/qemu/qemu.elf
	$(OBJCOPY) --update-section .image=tools/disk.img tools/qemu/qemu.elf
	$(RISCV_QEMU) -readconfig tools/qemu/sifive-e31.cfg -kernel tools/qemu/qemu.elf -nographic

clean:
	rm -rf build tools/qemu/qemu.elf tools/disk.img tools/bootROM.bin tools/mkfs tools/mkrom

GREEN = \033[1;32m
YELLOW = \033[1;33m
CYAN = \033[1;36m
END = \033[0m
