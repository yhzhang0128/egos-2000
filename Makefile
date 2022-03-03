all: apps
	@echo "$(GREEN)-------- Compile the Grass Layer --------$(END)"
	$(RISCV_CC) $(CFLAGS) $(LDFLAGS) $(GRASS_LAYOUT) $(GRASS_SRCS) $(DEFAULT_LDLIBS) $(INCLUDE) -o $(RELEASE_DIR)/grass.elf
	$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $(RELEASE_DIR)/grass.elf > $(DEBUG_DIR)/grass.lst
	@echo "$(YELLOW)-------- Compile the Earth Layer --------$(END)"
	$(RISCV_CC) $(CFLAGS) $(LDFLAGS) $(ARTY_FLAGS) $(EARTH_LAYOUT) $(EARTH_SRCS) $(EARTH_LDLIBS) $(INCLUDE) -o $(RELEASE_DIR)/earth.elf
	$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $(RELEASE_DIR)/earth.elf > $(DEBUG_DIR)/earth.lst

.PHONY: apps
apps: apps/*.c
	mkdir -p $(DEBUG_DIR) $(RELEASE_DIR)
	@echo "$(CYAN)-------- Compile the Apps Layer --------$(END)"
	for FILE in $^ ; do \
	  export APP=$$(basename $${FILE} .c);\
	  echo "Compile" $${FILE} "=>" $(RELEASE_DIR)/$${APP}.elf;\
	  $(RISCV_CC) $(CFLAGS) $(LDFLAGS) $(APPS_LAYOUT) $(APPS_SRCS) $${FILE} $(DEFAULT_LDLIBS) $(INCLUDE) -o $(RELEASE_DIR)/$${APP}.elf;\
	  echo "Compile" $${FILE} "=>" $(DEBUG_DIR)/$${APP}.lst;\
	  $(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $(RELEASE_DIR)/$${APP}.elf > $(DEBUG_DIR)/$${APP}.lst;\
	done

.PHONY: install
install:
	@echo "$(YELLOW)-------- Create the Disk Image --------$(END)"
	$(CC) $(BUILD_DIR)/mkfs.c shared/file/treedisk.c -DMKFS $(INCLUDE) -o $(BUILD_DIR)/mkfs
	cd $(BUILD_DIR); ./mkfs
	@echo "$(YELLOW)-------- Create the BootROM Image --------$(END)"
	$(OBJCOPY) -O binary $(RELEASE_DIR)/earth.elf $(BUILD_DIR)/earth.bin
	$(CC) $(BUILD_DIR)/mkrom.c -o $(BUILD_DIR)/mkrom
	cd $(BUILD_DIR); ./mkrom
	rm $(BUILD_DIR)/mkfs $(BUILD_DIR)/mkrom $(BUILD_DIR)/earth.bin 

clean:
	rm -rf $(DEBUG_DIR) $(RELEASE_DIR)
	rm -rf $(BUILD_DIR)/mkfs $(BUILD_DIR)/mkrom
	rm -rf $(BUILD_DIR)/disk.img $(BUILD_DIR)/bootROM.mcs
	rm -rf $(BUILD_DIR)/earth.bin $(BUILD_DIR)/*.log

EARTH_SRCS = earth/*.c earth/sd/*.c shared/*.c
EARTH_LAYOUT = -Tearth/layout.lds

GRASS_SRCS = grass/enter.S grass/*.c shared/*.c shared/*.S
GRASS_LAYOUT = -Tgrass/layout.lds

APPS_SRCS = apps/enter.S shared/*.c shared/file/*.c
APPS_LAYOUT = -Tapps/layout.lds

RISCV_CC = riscv64-unknown-elf-gcc
OBJDUMP = riscv64-unknown-elf-objdump
OBJCOPY = riscv64-unknown-elf-objcopy

BUILD_DIR = install
DEBUG_DIR = $(BUILD_DIR)/bin/debug
RELEASE_DIR = $(BUILD_DIR)/bin/release

INCLUDE = -Ishared/include -Ishared/file
CFLAGS = -march=rv32imac -mabi=ilp32 -mcmodel=medlow
CFLAGS += -ffunction-sections -fdata-sections

ARTY_FLAGS = -L$(BUILD_DIR)/bin
LDFLAGS = -Wl,--gc-sections -nostartfiles -nostdlib

DEFAULT_LDLIBS = -lc -lgcc
EARTH_LDLIBS = -Wl,--start-group -lc -lgcc -lm -lmetal -lmetal-gloss -Wl,--end-group

GREEN = \033[1;32m
YELLOW = \033[1;33m
CYAN = \033[1;36m
END = \033[0m
