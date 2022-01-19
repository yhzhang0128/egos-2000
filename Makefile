all:
	mkdir -p $(DEBUG_DIR) $(RELEASE_DIR)
	@echo "---------------- Compile the Earth Layer ----------------"
	$(CC) $(CFLAGS) $(LDFLAGS) $(ARTY_FLAGS) $(EARTH_LAYOUT) $(EARTH_SRCS) $(EARTH_LDLIBS) $(INCLUDE) -o $(RELEASE_DIR)/earth.elf
	$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $(RELEASE_DIR)/earth.elf > $(DEBUG_DIR)/earth.lst
	@echo "---------------- Compile the Grass Layer ----------------"
	$(CC) $(CFLAGS) $(LDFLAGS) $(GRASS_LAYOUT) $(GRASS_SRCS) $(DEFAULT_LDLIBS) $(INCLUDE) -o $(RELEASE_DIR)/grass.elf
	$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $(RELEASE_DIR)/grass.elf > $(DEBUG_DIR)/grass.lst

loc:
	cloc . --fullpath --not-match-d=earth/arty

clean:
	rm -rf $(BUILD_DIR)


EARTH_SRCS = earth/*.c earth/sdlib/*.c shared/*.c
EARTH_LAYOUT = -Tearth/layout.lds

GRASS_SRCS = grass/enter.S grass/*.c shared/*.c
GRASS_LAYOUT = -Tgrass/layout.lds


CC = riscv64-unknown-elf-gcc
OBJDUMP = riscv64-unknown-elf-objdump

BUILD_DIR = build
DEBUG_DIR = $(BUILD_DIR)/debug
RELEASE_DIR = $(BUILD_DIR)/release

INCLUDE = -Ishared/include
CFLAGS = -march=rv32imac -mabi=ilp32 -mcmodel=medlow
CFLAGS += -ffunction-sections -fdata-sections

ARTY_FLAGS = -Learth/arty -Iearth/arty
LDFLAGS = -Wl,--gc-sections -nostartfiles -nostdlib

DEFAULT_LDLIBS = -lc -lgcc
EARTH_LDLIBS = -Wl,--start-group -lc -lgcc -lm -lmetal -lmetal-gloss -Wl,--end-group
