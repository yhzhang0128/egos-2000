all:
	mkdir -p $(DEBUG_DIR) $(RELEASE_DIR)
	@echo "---------------- Compile the Apps Layer ----------------"
	$(RISCV_CC) $(CFLAGS) $(LDFLAGS) $(ECHO_LAYOUT) $(ECHO_SRCS) $(DEFAULT_LDLIBS) $(INCLUDE) -o $(RELEASE_DIR)/echo.elf
	$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $(RELEASE_DIR)/echo.elf > $(DEBUG_DIR)/echo.lst
	@echo "---------------- Compile the Grass Layer ----------------"
	$(RISCV_CC) $(CFLAGS) $(LDFLAGS) $(GRASS_LAYOUT) $(GRASS_SRCS) $(DEFAULT_LDLIBS) $(INCLUDE) -o $(RELEASE_DIR)/grass.elf
	$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $(RELEASE_DIR)/grass.elf > $(DEBUG_DIR)/grass.lst
	@echo "---------------- Compile the Earth Layer ----------------"
	$(RISCV_CC) $(CFLAGS) $(LDFLAGS) $(ARTY_FLAGS) $(EARTH_LAYOUT) $(EARTH_SRCS) $(EARTH_LDLIBS) $(INCLUDE) -o $(RELEASE_DIR)/earth.elf
	$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $(RELEASE_DIR)/earth.elf > $(DEBUG_DIR)/earth.lst
	cp $(RELEASE_DIR)/earth.elf $(BUILD_DIR)/earth.elf
	@echo "---------------- Create the Install Image ----------------"
	$(CC) install/mkfs.c -o $(BUILD_DIR)/mkfs
	cd install; ./mkfs
loc:
	cloc . --fullpath --not-match-d=earth/include/metal

clean:
	rm -rf $(DEBUG_DIR) $(RELEASE_DIR) $(BUILD_DIR)/mkfs $(BUILD_DIR)/disk.img $(BUILD_DIR)/earth.elf

EARTH_SRCS = earth/*.c earth/sd/*.c shared/*.c
EARTH_LAYOUT = -Tearth/layout.lds

GRASS_SRCS = grass/enter.S grass/*.c shared/*.c
GRASS_LAYOUT = -Tgrass/layout.lds

ECHO_SRCS = apps/enter.S apps/echo.c shared/*.c
ECHO_LAYOUT = -Tapps/layout.lds

RISCV_CC = riscv64-unknown-elf-gcc
OBJDUMP = riscv64-unknown-elf-objdump

BUILD_DIR = install
DEBUG_DIR = $(BUILD_DIR)/debug
RELEASE_DIR = $(BUILD_DIR)/release

INCLUDE = -Ishared/include
CFLAGS = -march=rv32imac -mabi=ilp32 -mcmodel=medlow
CFLAGS += -ffunction-sections -fdata-sections

ARTY_FLAGS = -Linstall/arty -Iinstall/arty
LDFLAGS = -Wl,--gc-sections -nostartfiles -nostdlib

DEFAULT_LDLIBS = -lc -lgcc
EARTH_LDLIBS = -Wl,--start-group -lc -lgcc -lm -lmetal -lmetal-gloss -Wl,--end-group
