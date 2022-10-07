all: apps
	@echo "$(GREEN)-------- Compile the Grass Layer --------$(END)"
	$(RISCV_CC) $(COMMON) $(GRASS_SRCS) $(GRASS_LD) -o $(RELEASE)/grass.elf
	$(OBJDUMP) $(OBJDUMP_FLAGS) $(RELEASE)/grass.elf > $(DEBUG)/grass.lst
	@echo "$(YELLOW)-------- Compile the Earth Layer --------$(END)"
	$(RISCV_CC) $(COMMON) $(EARTH_SRCS) $(EARTH_LD) -o $(RELEASE)/earth.elf
	$(OBJDUMP) $(OBJDUMP_FLAGS) $(RELEASE)/earth.elf > $(DEBUG)/earth.lst

.PHONY: apps
apps: apps/system/*.c apps/user/*.c
	mkdir -p $(DEBUG) $(RELEASE)
	@echo "$(CYAN)-------- Compile the Apps Layer --------$(END)"
	for FILE in $^ ; do \
	  export APP=$$(basename $${FILE} .c);\
	  echo "Compile" $${FILE} "=>" $(RELEASE)/$${APP}.elf;\
	  $(RISCV_CC) $(COMMON) $(APPS_SRCS) $${FILE} $(APPS_LD) -Iapps -o $(RELEASE)/$${APP}.elf || exit 1 ;\
	  echo "Compile" $${FILE} "=>" $(DEBUG)/$${APP}.lst;\
	  $(OBJDUMP) $(OBJDUMP_FLAGS) $(RELEASE)/$${APP}.elf > $(DEBUG)/$${APP}.lst;\
	done

install:
	@echo "$(YELLOW)-------- Create the Disk Image --------$(END)"
	$(CC) $(TOOLS)/mkfs.c library/file/file.c -DMKFS $(INCLUDE) -o $(TOOLS)/mkfs
	cd $(TOOLS); ./mkfs
	@echo "$(YELLOW)-------- Create the BootROM Image --------$(END)"
	cp $(RELEASE)/earth.elf $(TOOLS)/earth.elf
	$(OBJCOPY) --remove-section=.image $(TOOLS)/earth.elf
	$(OBJCOPY) -O binary $(TOOLS)/earth.elf $(TOOLS)/earth.bin
	$(CC) $(TOOLS)/mkrom.c -o $(TOOLS)/mkrom
	cd $(TOOLS); ./mkrom ; rm earth.elf earth.bin

program:
	@echo "$(YELLOW)-------- Program the on-board ROM --------$(END)"
	cd $(TOOLS)/fpga/openocd; time openocd -f 7series.txt

qemu:
	@echo "$(YELLOW)-------- Simulate on QEMU-RISCV --------$(END)"
	cp $(RELEASE)/earth.elf $(QEMU)/qemu.elf
	$(OBJCOPY) --update-section .image=$(TOOLS)/disk.img $(QEMU)/qemu.elf
	$(RISCV_QEMU) -readconfig $(QEMU)/sifive-e31.cfg -kernel $(QEMU)/qemu.elf -nographic

clean:
	rm -rf build
	rm -rf $(TOOLS)/qemu/qemu.elf
	rm -rf $(TOOLS)/mkfs $(TOOLS)/mkrom
	rm -rf $(TOOLS)/disk.img $(TOOLS)/bootROM.bin $(TOOLS)/bootROM.mcs

RISCV_QEMU = qemu-system-riscv32
RISCV_CC = riscv64-unknown-elf-gcc
OBJDUMP = riscv64-unknown-elf-objdump
OBJCOPY = riscv64-unknown-elf-objcopy

APPS_SRCS = apps/app.S library/*/*.c
GRASS_SRCS = grass/grass.S grass/*.c library/elf/*.c
EARTH_SRCS = earth/earth.S earth/*.c earth/sd/*.c library/elf/*.c library/libc/*.c

CFLAGS = -march=rv32i -mabi=ilp32 -mcmodel=medlow -ffunction-sections -fdata-sections
LDFLAGS = -Wl,--gc-sections -nostartfiles -nostdlib
INCLUDE = -Ilibrary -Ilibrary/elf -Ilibrary/libc -Ilibrary/file -Ilibrary/servers

COMMON = $(CFLAGS) $(LDFLAGS) $(INCLUDE) -D CPU_CLOCK_RATE=65000000

APPS_LD = -Tapps/app.lds -lc -lgcc
GRASS_LD = -Tgrass/grass.lds -lc -lgcc
EARTH_LD = -Tearth/earth.lds -lc -lgcc

TOOLS = tools
QEMU = tools/qemu
DEBUG = build/debug
RELEASE = build/release
OBJDUMP_FLAGS =  --source --all-headers --demangle --line-numbers --wide

GREEN = \033[1;32m
YELLOW = \033[1;33m
CYAN = \033[1;36m
END = \033[0m
