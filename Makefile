all: app
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

app: apps/*.c
	mkdir -p $(DEBUG_DIR) $(RELEASE_DIR)
	@echo "---------------- Compile the Apps Layer ----------------"
	for FILE in $^ ; do \
	  export APP_NAME=$$(basename $${FILE} .c);\
	  echo "Compile" $${FILE} "=>" $(RELEASE_DIR)/$${APP_NAME}.elf;\
	  $(RISCV_CC) $(CFLAGS) $(LDFLAGS) $(APPS_LAYOUT) $(APPS_SRCS) $${FILE} $(DEFAULT_LDLIBS) $(INCLUDE) -o $(RELEASE_DIR)/$${APP_NAME}.elf;\
	  echo "Compile" $${FILE} "=>" $(DEBUG_DIR)/$${APP_NAME}.lst;\
	  $(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $(RELEASE_DIR)/$${APP_NAME}.elf > $(DEBUG_DIR)/$${APP_NAME}.lst;\
	done

loc:
	cloc . --exclude-dir=install

clean:
	rm -rf $(DEBUG_DIR) $(RELEASE_DIR) $(BUILD_DIR)/mkfs $(BUILD_DIR)/disk.img $(BUILD_DIR)/earth.elf

EARTH_SRCS = earth/*.c earth/sd/*.c shared/*.c
EARTH_LAYOUT = -Tearth/layout.lds

GRASS_SRCS = grass/enter.S grass/*.c shared/*.c
GRASS_LAYOUT = -Tgrass/layout.lds

APPS_SRCS = apps/enter.S shared/*.c
APPS_LAYOUT = -Tapps/layout.lds

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
