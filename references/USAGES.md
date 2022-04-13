# Compile and run egos-2000

You can use Windows, Linux or MacOS. For Windows users, use WSL (Windows Subsystem for Linux) for step1 and step2.

## Step1: Setup the compiler and compile egos-2000

Setup your working directory and name it as `$EGOS`.

```shell
> export EGOS=/home/yunhao/egos
> cd $EGOS
> git clone https://github.coecis.cornell.edu/4411-riscv/egos-2000.git
# now the code repository is at $EGOS/egos-2000
```

Download the SiFive [riscv-gcc compiler](https://github.com/sifive/freedom-tools/releases/tag/v2020.04.0-Toolchain.Only) to the working directory.

```shell
> cd $EGOS
> tar -zxvf riscv64-unknown-elf-gcc-8.3.0-2020.04.1-x86_64-xxx-xxx.tar.gz
> export PATH=$PATH:$EGOS/riscv64-unknown...../bin
> cd $EGOS/egos-2000
> make
mkdir -p build/debug build/release
-------- Compile the Apps Layer --------
......
-------- Compile the Grass Layer --------
......
-------- Compile the Earth Layer --------
......
```


`build/release` holds the ELF format binary executables and `build/debug` holds the human readable assembly files.

## Step2: Create the disk and bootROM images

```shell
> cd $EGOS/egos-2000
> make install
-------- Create the Disk Image --------
......
[INFO] Finish making the disk image
-------- Create the BootROM Image --------
......
[INFO] Finish making the bootROM binary
[INFO] Finish making the bootROM mcs image
```

This will create `disk.img`, `bootROM.bin` and `bootROM.mcs` in the `tools` directory.
To program `disk.img` to the microSD card:

```shell
# Find your microSD card in /dev
> sudo fdisk -l
......
Disk /dev/sdb: 117.8 GiB, 126437294080 bytes, 246947840 sectors
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
...
# Say it is /dev/sdb as shown above
> sudo dd if=$EGOS/egos-2000/tools/disk.img of=/dev/sdb
8192+0 records in
8192+0 records out
4194304 bytes (4.2 MB, 4.0 MiB) copied, 0.377515 s, 11.1 MB/s
```

You can also use GUI softwares like [balena Etcher](https://www.balena.io/etcher/) to program `disk.img` to your microSD card.

## Step3: Program the Arty board

### Windows and Linux
Install Vivado Lab Edition which can be downloaded [here](https://www.xilinx.com/support/download.html) or [here](https://drive.google.com/file/d/1VS6_mxb6yrAxdDtlXkHdB-8jg9CScacw/view?usp=sharing). You may need to register a Xilinx account, but the software is free.

1. Connect the Arty FPGA board to your computer with the USB cable
2. Open Vivado Lab Edition
3. Click "Open Hardware Manager"
4. Click "Open target" and "Auto Connect"; the Arty board should appear in the "Hardware" window
5. In the "Hardware" window, right click `xc7a35t` and click "Add Configuration Memory Device"
6. Choose memory device "mt25ql128-spi-x1_x2_x4" and click "Program Configuration Memory Device"
7. In the "Configuration file" field, choose the `bootROM.mcs` file compiled in step 2
8. Click "OK" and wait for the program to finish
9. Press the `program` red button on the left-top corner of the Arty board
10. For Linux users, type in your shell
```shell
> sudo chmod 666 /dev/ttyUSB1
> screen /dev/ttyUSB1 115200
```
11. For Mac users, use the same commands but check your `/dev` directory for the serial device name
12. For Windows users, use software like `PuTTY` to connect with the serial port (e.g., COM6) and use baud rate 115200

To restart egos, press the `reset` red button on the right-top corner of the Arty board.

In step4, if the Arty board is not detected, try to reinstall the USB cable drivers following [these instructions](https://support.xilinx.com/s/article/59128?language=en_US). If it still doesn't work, it may be an issue with Vivado and please contact Xilinx [here](https://support.xilinx.com/s/topic/0TO2E000000YKXgWAO/programmable-logic-io-bootconfiguration?language=en_US).

In step6, new versions of Arty may use "s25fl128sxxxxxx0" as memory device. 
If you choose the wrong one, step8 will tell you.

### MacOS

Install [Homebrew](https://brew.sh/).
Then install `openocd` using Homebrew by typing `brew install openocd` in your terminal. 

Prepare a text file `7series.txt` with the following content. 
Replace the `$(EGOS_DIR)` below with your own egos-2000 path.

```
# File: 7series.txt
interface ftdi
ftdi_device_desc "Digilent USB Device"
ftdi_vid_pid 0x0403 0x6010

# channel 1 does not have any functionality
ftdi_channel 0

# just TCK TDI TDO TMS, no reset
ftdi_layout_init 0x0088 0x008b
reset_config none
adapter_khz 10000

source [find cpld/xilinx-xc7.cfg]
source [find cpld/jtagspi.cfg]

init

puts [irscan xc7.tap 0x09]
puts [drscan xc7.tap 32 0]  

puts "Programming FPGA..."

jtagspi_init 0 bscan_spi_xc7a35t.bit
jtagspi_program $(EGOS_DIR)/tools/bootROM.bin 0 

exit
```

Download file `bscan_spi_xc7a35t.bit` from [here](https://github.com/quartiq/bscan_spi_bitstreams/blob/master/bscan_spi_xc7a35t.bit) and put it in the same directory as `7series.txt`.
In your terminal, type `openocd -f 7series.txt` and wait for about 3 minutes until the program finishes.
Then refer to step9-13 in the Windows/Linux instructions above.

This method is borrowed from [this wiki](https://github.com/byu-cpe/BYU-Computing-Tutorials/wiki/Program-7-Series-FPGA-from-a-Mac-or-Linux-Without-Xilinx?_ga=2.208554260.708413845.1647041461-635131311.1640671103).
