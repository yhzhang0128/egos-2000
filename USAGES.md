# Compile and run egos-2000

You can use MacOS, Linux or Windows and here are the tutorial videos:
[MacOS](https://youtu.be/VJgQFcKG0uc), [Linux](https://youtu.be/2FT7AN0wPlg) and [Windows](https://youtu.be/hCDMnGGyGqM).
MacOS users can follow the same tutorial no matter you have an Apple chip or Intel CPU.
You can run egos-2000 on the QEMU emulator or RISC-V boards.
Using QEMU is easier, but if you wish to run it on real hardware for fun, 
you have 2 options:
1. [Sipeed Tang Nano 20K](https://wiki.sipeed.com/hardware/en/tang/tang-nano-20k/nano-20k.html), a [microSD reader](https://www.amazon.com/dp/B07G5JV2B5?psc=1&ref=ppx_yo2_dt_b_product_details), and a microSD card
2. Arty [A7-35t](https://www.xilinx.com/products/boards-and-kits/arty.html)/[A7-100t](https://digilent.com/shop/arty-a7-100t-artix-7-fpga-development-board/)/[S7-50](https://digilent.com/shop/arty-s7-spartan-7-fpga-development-board/),
a [VGA Pmod](https://digilent.com/reference/pmod/pmodvga/start),
an [ESP32 Pmod](https://digilent.com/reference/pmod/pmodesp32/start),
a [microSD Pmod](https://digilent.com/reference/pmod/pmodmicrosd/start?redirect=1), a [microSD reader](https://www.amazon.com/dp/B07G5JV2B5?psc=1&ref=ppx_yo2_dt_b_product_details), and a microSD card

The option of Tang Nano 20K is cheaper, but it does not support multicore, Ethernet, or Wi-Fi.
It supports microSD and HDMI though.

## Step1: Setup the compiler and compile egos-2000

Setup your working directory and name it as `$EGOS`.

```shell
> export EGOS=/home/yunhao/egos
> cd $EGOS
> git clone https://github.com/yhzhang0128/egos-2000.git
```
Download the [pre-built binaries of the RISC-V GNU compiler toolchain](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases/tag/v14.2.0-3) from xPack to `$EGOS` and compile egos-2000.

```shell
> cd $EGOS
> tar -zxvf xpack-riscv-none-elf-gcc-14.2.0-3-{linux,darwin}-{x64,arm64}.tar.gz
> export PATH=$PATH:$EGOS/xpack-riscv-none-elf-gcc-14.2.0-3/bin
> cd $EGOS/egos-2000
> make
......
```

## Step2: Run egos-2000 on the QEMU emulator

Download the [pre-built binaries of QEMU](https://github.com/xpack-dev-tools/qemu-riscv-xpack/releases/tag/v8.2.2-1) from xPack.

```shell
> cd $EGOS
> tar -zxvf xpack-qemu-riscv-8.2.2-1-xxxxxx.tar.gz
> export PATH=$PATH:$EGOS/xpack-qemu-riscv-8.2.2-1-xxxxxx/bin
> cd $EGOS/egos-2000
> make qemu
-------- Simulate on QEMU-RISCV --------
qemu-system-riscv32 -nographic -readconfig tools/qemu/config.toml
[CRITICAL] --- Booting on QEMU with core #4 ---
[INFO] Set the CS pin to HIGH and toggle clock
[INFO] Set the CS pin to LOW and send cmd0 to SD card
[INFO] Check SD card type and voltage with cmd8
[INFO] SD card replies cmd8 with status 1
[SUCCESS] Finished initializing the tty and disk devices
[CRITICAL] Choose a memory translation mechanism:
Enter 0: page tables
Enter 1: software TLB
```

## Step3: Run egos-2000 on a board

You can use the Tang Nano 20K, Arty A7-35t, A7-100t, or S7-50 board.
To use a microSD card on the board, program the microSD card with `disk.img` using tools like [balena Etcher](https://www.balena.io/etcher/).

### Step3.1: MacOS or Linux

Install **openFPGALoader** with [Homebrew](https://formulae.brew.sh/formula/openfpgaloader) on MacOS or [this guide](https://wiki.sipeed.com/hardware/en/tang/Tang-Nano-Doc/flash-in-linux.html) on Linux.
The `BOARD` in the command below can be `arty_a7_35t`, `arty_a7_100t`, `arty_s7_50`, or `tangnano20k`.

```shell
> cd $EGOS/egos-2000
> make program BOARD=tangnano20k
-------- Program the tangnano20k on-board ROM --------
openFPGALoader -b tangnano20k -f tools/bootROM.bin
......
start addr: 00000000, end_addr: 00800000
Erasing: [==================================================] 100.00%
Done
Writing: [==================================================] 100.00%
Done
```

To connect with the egos-2000 TTY:

```shell
> sudo chmod 666 /dev/ttyUSB1
> screen /dev/ttyUSB1 115200
......
--============== Boot ==================--
[INFO] LiteX + VexRiscv (vendorid: 666)
[INFO] Press 'b' to enter BIOS instead of EGOS
[INFO] Initializing SD card
[INFO] Reading 256 blocks (128 KB) to 0x8000_0000
[INFO] Jumping to 0x8000_0000
[CRITICAL] --- Booting on Hardware with core #0 ---
......
```
For MacOS users, check your `/dev` directory for the TTY device name (e.g., `/dev/tty.usbserial-xxxxxx`).
To reboot egos-2000, press one of the two white buttons on Tang Nano 20K or the top-right `RESET` button on the Arty boards.

### Step3.2: Windows (only for the Arty boards)

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

![This is an image](tools/screenshots/vivado.png)

Lastly, to connect with the egos-2000 TTY, find your board in the "Device Manager" (e.g., COM6) and use `PuTTY` to connect:

![This is an image](tools/screenshots/putty.png)
