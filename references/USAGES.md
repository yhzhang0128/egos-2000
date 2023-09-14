# Compile and run egos-2000

You can use MacOS, Linux or Windows and here are the tutorial videos:
[MacOS](https://youtu.be/VJgQFcKG0uc), [Linux](https://youtu.be/2FT7AN0wPlg) and [Windows](https://youtu.be/hCDMnGGyGqM).
MacOS users can follow the same tutorial no matter you have an Apple chip or Intel CPU.
You can run egos-2000 on the QEMU emulator or RISC-V boards.
Running on QEMU is easier but if you wish to run it on the boards for fun, 
you need to purchase the following hardware:
* one of the Arty [A7-35T board](https://www.xilinx.com/products/boards-and-kits/arty.html), [A7-100T board](https://digilent.com/shop/arty-a7-100t-artix-7-fpga-development-board/) and [S7-50 board](https://digilent.com/shop/arty-s7-spartan-7-fpga-development-board/)
* a microUSB cable (e.g., [microUSB-to-USB-C](https://www.amazon.com/dp/B0744BKDRD?psc=1&ref=ppx_yo2_dt_b_product_details))
* [optional] a [microSD Pmod](https://digilent.com/reference/pmod/pmodmicrosd/start?redirect=1), a [microSD reader](https://www.amazon.com/dp/B07G5JV2B5?psc=1&ref=ppx_yo2_dt_b_product_details) and a microSD card


## Step1: Setup the compiler and compile egos-2000

Setup your working directory and name it as `$EGOS`.

```shell
> export EGOS=/home/yunhao/egos
> cd $EGOS
> git clone https://github.com/yhzhang0128/egos-2000.git
```

Download the [GNU toolchain from SiFive](https://github.com/sifive/freedom-tools/releases/tag/v2020.04.0-Toolchain.Only) to `$EGOS` and compile egos-2000.

```shell
> cd $EGOS
> tar -zxvf riscv64-unknown-elf-gcc-8.3.0-2020.04.1-x86_64-xxx-xxx.tar.gz
> export PATH=$PATH:$EGOS/riscv64-unknown...../bin
> cd $EGOS/egos-2000
> make
......
```

After this step, `build/release` holds the ELF format executables and `build/debug` holds the human readable assembly files.
While using the SiFive toolchain is easier, you can also compile and install the [official GNU toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain) which takes about an hour.

```shell
# Prepare the environment
> sudo apt-get install autoconf automake autotools-dev curl python3 python3-pip libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev ninja-build git cmake libglib2.0-dev
> cd $EGOS
> mkdir riscv32-unknown-elf-gcc
> export PATH=$PATH:$EGOS/riscv32-unknown-elf-gcc/bin

# Compile and install the GNU toolchain
> git clone git@github.com:riscv-collab/riscv-gnu-toolchain.git
> cd riscv-gnu-toolchain
> ./configure --with-arch=rv32imac --with-abi=ilp32 --prefix=$EGOS/riscv32-unknown-elf-gcc
> make
......
# Compile egos-2000 with the official GNU toolchain
> cd $EGOS/egos-2000
> make TOOLCHAIN=GNU
```

## Step2: Create the disk and bootROM images

Make sure you have a C compiler (i.e., the `cc` command) in your shell environment.

```shell
> cd $EGOS/egos-2000
> make install
-------- Create the Disk Image --------
......
[INFO] Finish making the disk image (tools/disk.img)
-------- Create the BootROM Image --------
......
[INFO] Finish making the bootROM binary (tools/bootROM.bin)
```

This will create `disk.img` and `bootROM.bin` under the `tools` directory.

## Step3: Run egos-2000 on the QEMU emulator

Download [QEMU v5.2](https://github.com/yhzhang0128/freedom-tools/releases/tag/v2023.9.8) for egos-2000 to the working directory `$EGOS`.
You can also compile and install [QEMU v8.1](https://github.com/yhzhang0128/qemu/releases/tag/v8.1.0-egos) yourself.

```shell
> cd $EGOS
> tar -zxvf riscv-qemu-5.2.0-xxxxxx.tar.gz
> export PATH=$PATH:$EGOS/riscv-qemu-5.2.0-xxxxxx/bin
> cd $EGOS/egos-2000
> make qemu
-------- Simulate on QEMU-RISCV --------
cp build/release/earth.elf tools/qemu/qemu.elf
riscv64-unknown-elf-objcopy --update-section .image=tools/disk.img tools/qemu/qemu.elf
qemu-system-riscv32 -readconfig tools/qemu/sifive-e31.cfg -kernel tools/qemu/qemu.elf -nographic
[CRITICAL] --- Booting on QEMU ---
......
```

## Step4: Run egos-2000 on the Arty board

You can use the Arty A7-35t, A7-100t or S7-50 board
and make sure to set the `BOARD` variable in `Makefile` correctly.
To use a microSD card on the board, you can program the microSD card with `disk.img` using tools like [balena Etcher](https://www.balena.io/etcher/).

### Step4.1: MacOS or Linux

Download [OpenOCD v0.11.0-1](https://github.com/xpack-dev-tools/openocd-xpack/releases/tag/v0.11.0-1) to `$EGOS`
and program `bootROM.bin` to the on-board ROM.

```shell
> cd $EGOS
> tar -zxvf xpack-openocd-0.11.0-1-xxx-xxx.tar.gz
> export PATH=$PATH:$EGOS/xpack-openocd-0.11.0-1-xxx-xxx/bin
> cd $EGOS/egos-2000
> make program
-------- Program the on-board ROM --------
cd tools/openocd; time openocd -f 7series.txt
......
Info : sector 190 took 229 ms
Info : sector 191 took 243 ms  # It will pause at this point for a while
Info : Found flash device 'micron n25q128' (ID 0x0018ba20)

real    2m51.019s
user    0m11.540s
sys     0m37.338s

```

To connect with the egos-2000 TTY:

1. Press the `PROG` red button on the left-top corner of the Arty board
2. To restart, press the `RESET` red button on the right-top corner
3. For Linux users, type in your shell
```shell
> sudo chmod 666 /dev/ttyUSB1
> screen /dev/ttyUSB1 115200
[CRITICAL] --- Booting on Arty ---
......
```
4. For MacOS users, check your `/dev` directory for the TTY device name (e.g., `/dev/tty.usbserial-xxxxxx`)

### Step4.2: Windows

Install Vivado Lab Edition which can be downloaded [here](https://www.xilinx.com/support/download.html).
You may need to register a Xilinx account, but the software is free.

1. Open Vivado Lab Edition and click "Open Hardware Manager"
2. Click "Open target" and "Auto Connect"; the Arty board should appear in the "Hardware" window
3. In the "Hardware" window, right click `xc7a35t` and click "Add Configuration Memory Device"
4. Choose memory device "mt25ql128-spi-x1_x2_x4" and click "Program Configuration Memory Device"
5. In the "Configuration file" field, choose the `bootROM.bin` file created in step 2; Note that the [tutorial video](https://youtu.be/hCDMnGGyGqM) has shown how to generate this `bootROM.bin` file using Docker in Windows
6. Click "OK" and wait for the program to finish

In **2**, if the Arty board doesn't appear, try to install [Digilent Adept](https://digilent.com/reference/software/adept/start) or reinstall the USB cable drivers following [this post](https://support.xilinx.com/s/article/59128?language=en_US).

In **4**, some Arty boards may use "s25fl128sxxxxxx0" or other memory device. If you choose the wrong one, **6** will tell you.

![This is an image](screenshots/vivado.png)

Lastly, to connect with the egos-2000 TTY, find your board in the "Device Manager" (e.g., COM6) and use `PuTTY` to connect:

![This is an image](screenshots/putty.png)
