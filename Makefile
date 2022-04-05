all: apps
	@echo "$(GREEN)-------- Compile the Grass Layer --------$(END)"
	$(RISCV_CC) $(CFLAGS) $(LDFLAGS) $(CLOCK) $(GRASS_LAYOUT) $(GRASS_SRCS) $(DEFAULT_LDLIBS) $(INCLUDE) -o $(RELEASE_DIR)/grass.elf
	$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $(RELEASE_DIR)/grass.elf > $(DEBUG_DIR)/grass.lst
	@echo "$(YELLOW)-------- Compile the Earth Layer --------$(END)"
	$(RISCV_CC) $(CFLAGS) $(LDFLAGS) $(CLOCK) $(EARTH_LAYOUT) $(EARTH_SRCS) $(EARTH_LDLIBS) $(INCLUDE) -o $(RELEASE_DIR)/earth.elf
	$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $(RELEASE_DIR)/earth.elf > $(DEBUG_DIR)/earth.lst

.PHONY: apps
apps: apps/system/*.c apps/user/*.c
	mkdir -p $(DEBUG_DIR) $(RELEASE_DIR)
	@echo "$(CYAN)-------- Compile the Apps Layer --------$(END)"
	for FILE in $^ ; do \
	  export APP=$$(basename $${FILE} .c);\
	  echo "Compile" $${FILE} "=>" $(RELEASE_DIR)/$${APP}.elf;\
	  $(RISCV_CC) $(CFLAGS) $(LDFLAGS) $(CLOCK) $(APPS_LAYOUT) $(APPS_SRCS) $${FILE} $(DEFAULT_LDLIBS) $(INCLUDE) -Iapps -o $(RELEASE_DIR)/$${APP}.elf || exit 1 ;\
	  echo "Compile" $${FILE} "=>" $(DEBUG_DIR)/$${APP}.lst;\
	  $(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $(RELEASE_DIR)/$${APP}.elf > $(DEBUG_DIR)/$${APP}.lst;\
	done

.PHONY: install
install:
	@echo "$(YELLOW)-------- Create the Disk Image --------$(END)"
	$(CC) $(TOOLS_DIR)/mkfs.c library/file/file.c -DMKFS $(INCLUDE) -o $(TOOLS_DIR)/mkfs
	cd $(TOOLS_DIR); ./mkfs
	@echo "$(YELLOW)-------- Create the BootROM Image --------$(END)"
	$(OBJCOPY) -O binary $(RELEASE_DIR)/earth.elf $(TOOLS_DIR)/earth.bin
	$(CC) $(TOOLS_DIR)/mkrom.c -o $(TOOLS_DIR)/mkrom
	cd $(TOOLS_DIR); ./mkrom ; rm earth.bin

clean:
	rm -rf build
	rm -rf $(TOOLS_DIR)/mkfs $(TOOLS_DIR)/mkrom
	rm -rf $(TOOLS_DIR)/disk.img $(TOOLS_DIR)/bootROM.bin $(TOOLS_DIR)/bootROM.mcs

RISCV_CC = riscv64-unknown-elf-gcc
OBJDUMP = riscv64-unknown-elf-objdump
OBJCOPY = riscv64-unknown-elf-objcopy

CLOCK = -D CPU_CLOCK_RATE=65000000

EARTH_SRCS = earth/*.c earth/sd/*.c library/elf/*.c
EARTH_LAYOUT = -Tlibrary/sifive/metal.lds

GRASS_SRCS = grass/*.S grass/*.c library/elf/*.c
GRASS_LAYOUT = -Tgrass/grass.lds

APPS_SRCS = apps/app.S library/*/*.c
APPS_LAYOUT = -Tapps/app.lds

TOOLS_DIR = tools
DEBUG_DIR = build/debug
RELEASE_DIR = build/release

INCLUDE = -Ilibrary
INCLUDE += -Ilibrary/elf -Ilibrary/malloc -Ilibrary/file -Ilibrary/syscall
CFLAGS = -march=rv32imac -mabi=ilp32 -mcmodel=medlow
CFLAGS += -ffunction-sections -fdata-sections

DEFAULT_LDLIBS = -lc -lgcc
LDFLAGS = -Wl,--gc-sections -nostartfiles -nostdlib
EARTH_LDLIBS = -Llibrary/sifive -Wl,--start-group -lc -lgcc -lm -lmetal -lmetal-gloss -Wl,--end-group

GREEN = \033[1;32m
YELLOW = \033[1;33m
CYAN = \033[1;36m
END = \033[0m
