all:
	mkdir -p release
	@echo "---------------- Compile the Grass Kernel ----------------"
	$(CC) $(CFLAGS) $(LDFLAGS) $(GRASS_LDFLAGS) $(GRASS_SRCS) $(LDLIBS) -o release/grass.elf
	$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide ./release/grass.elf > ./release/grass.lst


GRASS_SRCS = grass/enter.S lib/malloc.c grass/grass.c

CC = riscv64-unknown-elf-gcc
OBJDUMP = riscv64-unknown-elf-objdump

LDFLAGS = -Wl,--gc-sections -nostartfiles -nostdlib
GRASS_LDFLAGS += -Tlayout/grass.lds

CFLAGS = -march=rv32imac -mabi=ilp32 -mcmodel=medlow
CFLAGS += -ffunction-sections -fdata-sections

LDLIBS = -Wl,--start-group -lc -lgcc -lm -Wl,--end-group
