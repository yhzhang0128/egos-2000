# Compile and Run EGOS on Arty

You can use Windows, Linux or MacOS. For Windows users, use WSL (Windows Subsystem for Linux) for step1 and step2.

## Step1: Setup compiler and compile egos-riscv

Setup your working directory and name it as `$ARTY`.

```shell
> export ARTY=/home/yunhao/arty
> cd $ARTY
> git clone git@github.coecis.cornell.edu:4411-riscv/egos-riscv.git
# now the code repository is at $ARTY/egos-riscv
```

Download the SiFive [riscv-gcc compiler](https://github.com/sifive/freedom-tools/releases/tag/v2020.04.0-Toolchain.Only) to the working directory.

```shell
> cd $ARTY
> tar -zxvf riscv64-unknown-elf-gcc-8.3.0-2020.04.1-x86_64-xxx-xxx.tar.gz
> export PATH=$PATH:$ARTY/riscv64-unknown...../bin
> cd $ARTY/egos-riscv
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
> cd $ARTY/egos-riscv
> make install
-------- Create the Disk Image --------
......
[INFO] Finish making the disk image
-------- Create the BootROM Image --------
......
[INFO] Finish making the bootROM binary
[INFO] Finish making the bootROM mcs image
```

This will create `disk.img`, `bootROM.bin` and `bootROM.mcs` in the `utils` directory.
You can use tools like [balena Etcher](https://www.balena.io/etcher/) to program `disk.img` to your microSD.

## Step3: Program the Arty FPGA board

### Windows and Linux
Install Vivado Lab Edition which can be downloaded [here](https://drive.google.com/file/d/1VS6_mxb6yrAxdDtlXkHdB-8jg9CScacw/view?usp=sharing) or [here](https://www.xilinx.com/support/download.html). You may need to register a Xilinx account, but the software is free.

1. Connect the Arty FPGA board to your computer with the USB cable
2. Open Vivado Lab Edition
3. Click "Open Hardware Manager"
4. Click "Open target" and "Auto Connect"; the Arty board should appear in the "Hardware" window
5. In the "Hardware" window, right click `xc7a35t` and click "Add Configuration Memory Device"
6. Choose memory device "s25fl128sxxxxxx0" and click "Program Configuration Memory Device"
7. In the "Configuration file" field, choose the `bootROM.mcs` file compiled in step 2
8. Click "OK" and wait for the program to finish
9. Click the `program` red button on the left-top corner of the Arty FPGA board
10. Click the `reset` red button on the right-top corner of the Arty FPGA board, for rebooting EGOS
11. For Linux users, type in your shell
```shell
> sudo chmod 666 /dev/ttyUSB1
> screen /dev/ttyUSB1 115200
```
12. For Mac users, use the same commands but check your `/dev` directory for the serial device name
13. For Windows users, use software like `PuTTY` to connect with the serial port using baud rate 115200

In step4, if the Arty board is not detected, try to reinstall the USB cable drivers following [these instructions](https://support.xilinx.com/s/article/59128?language=en_US).

In step6, old versions of Arty may use "mt25ql128-spi-x1_x2_x4" as memory device. 
If you choose the wrong one, step8 will tell you.

### MacOS

Install [Homebrew](https://brew.sh/).
Then install `openocd` using Homebrew by typing `brew install openocd` in your terminal. 

Prepare a text file `7series.txt` with the following content. 
Replace the `$(EGOS_RISCV_DIR)` with your own egos-riscv path.

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
jtagspi_program $(EGOS_RISCV_DIR)/utils/bootROM.bin 0 

exit
```

Download file `bscan_spi_xc7a35t.bit` from [here](https://github.com/quartiq/bscan_spi_bitstreams/blob/master/bscan_spi_xc7a35t.bit) and put it in the same directory as `7series.txt`.
In your terminal, type `openocd -f 7series.txt` and wait for about 3 minutes until the program finishes.
Then refer to step9-13 in the Windows/Linux instructions above.

This method is borrowed from [this wiki](https://github.com/byu-cpe/BYU-Computing-Tutorials/wiki/Program-7-Series-FPGA-from-a-Mac-or-Linux-Without-Xilinx?_ga=2.208554260.708413845.1647041461-635131311.1640671103).
