all:
	mkdir -p release
	@echo "---------------- Compile the Earth Layer ----------------"
	$(CC) $(CFLAGS) $(LDFLAGS) $(ARTY_FLAGS) $(EARTH_LAYOUT) $(EARTH_SRCS) $(EARTH_LDLIBS) -o release/earth.elf
	$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide ./release/earth.elf > ./release/earth.lst
	@echo "---------------- Compile the Grass Layer ----------------"
	$(CC) $(CFLAGS) $(LDFLAGS) $(GRASS_LAYOUT) $(GRASS_SRCS) $(DEFAULT_LDLIBS) -o release/grass.elf
	$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide ./release/grass.elf > ./release/grass.lst


EARTH_SRCS = earth/earth.c
EARTH_LAYOUT = -Tearth/layout.lds
ARTY_FLAGS = -Learth/arty -Iearth/arty

GRASS_SRCS = grass/enter.S lib/malloc.c grass/grass.c
GRASS_LAYOUT = -Tgrass/layout.lds


CC = riscv64-unknown-elf-gcc
OBJDUMP = riscv64-unknown-elf-objdump

CFLAGS = -march=rv32imac -mabi=ilp32 -mcmodel=medlow
CFLAGS += -ffunction-sections -fdata-sections

LDFLAGS = -Wl,--gc-sections -nostartfiles -nostdlib

DEFAULT_LDLIBS = -Wl,--start-group -lc -lgcc -lm -Wl,--end-group
EARTH_LDLIBS = -Wl,--start-group -lc -lgcc -lm -lmetal -lmetal-gloss -Wl,--end-group

