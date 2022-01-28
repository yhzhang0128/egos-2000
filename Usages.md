# Compile and Run EGOS on Arty

You can use Linux, Mac or Windows to compile egos-riscv.
But Vivado is needed to program the bootROM image to the Arty board and Vivado only supports Linux and Windows.

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
> cd $ARTY/egos-riscv
> tar -zxvf riscv64-unknown-elf-gcc-8.3.0-2020.04.1-x86_64-xxx-xxx.tar.gz
> export PATH=$PATH:$ARTY/riscv64-unknown...../bin
> make
mkdir -p install/exec/debug install/exec/release
-------- Compile the Apps Layer --------
......
-------- Compile the Grass Layer --------
......
-------- Compile the Earth Layer --------
......
```

This will create two directories `debug` and `release` in `install/exec`. 
`release` holds the ELF format binary executables and `debug` holds the human readable assembly files.

## Step2: Create the disk and bootROM images

```shell
> cd $ARTY/egos-riscv
> make install
-------- Create the Disk Image --------
......
-------- Create the BootROM Image --------
......
```

This will create `disk.img` and `bootROM.mcs` in `install`.
You can use tools like [balena Etcher](https://www.balena.io/etcher/) to program `disk.img` to your microSD.

## Step3: Program the Arty FPGA board

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

## Appendix: Modify and recompile the processor

In rare cases, you may want to modify the FE310 RISC-V processor.
For example, we have modified the processor to increase the memory size from 32KB to 160KB and connect its SPI bus controller to the microSD card.

We prepared a docker image encapsulating the toolchain environment.

```shell
# This image takes ~45GB of disk space.
> docker pull yhzhang0128/arty-toolchain:latest
> docker run -it \
	--mount type=bind,source=$ARTY,target=/root/host \
	yhzhang0128/arty-toolchain:latest
# enter the shell of docker container
# $ARTY on host is mapped to /root/host in the docker container

root@579c433a6161:/# cd /root
root@579c433a6161:/# ls
fe310-cpu  fe310-deps  fe310-sdk  host
root@579c433a6161:/# cd /root/fe310-cpu
# do your modifications in this directory

# compile the processor
root@579c433a6161:/# ./4411.sh
......
......
===================================
Configuration Memory information
===================================
File Format        MCS
Interface          SPIX4
Size               16M
Start Address      0x00000000
End Address        0x00FFFFFF

Addr1         Addr2         Date                    File(s)
0x00000000    0x0021728B    Jan 26 02:14:02 2022    /root/fe310-cpu/builds/e300artydevkit/obj/E300ArtyDevKitFPGAChip.bit
0x00400000    0x00415047    Jan 27 16:45:46 2022    /root/fe310-cpu/earth.bin
0x00800000    0x009FFFFF    Jan 26 01:52:57 2022    /root/fe310-cpu/disk.img
0 Infos, 0 Warnings, 0 Critical Warnings and 0 Errors encountered.
write_cfgmem completed successfully
INFO: [Common 17-206] Exiting Vivado at Thu Jan 27 16:45:59 2022...


# copy the new processor binary from docker
# to the egos-riscv repository in the host machine
root@579c433a6161:/# cp builds/e300artydevkit/obj/E300ArtyDevKitFPGAChip.bit /root/host/.../egos-riscv/install/arty_board/fe310_cpu.bit
```
