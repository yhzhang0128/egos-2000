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

Download the [SiFive riscv-gcc compiler](https://github.com/sifive/freedom-tools/releases/tag/v2020.04.0-Toolchain.Only) to the working directory `$EGOS`.

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
To program `disk.img` to a microSD card:

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

## Step3: Program the Arty on-board ROM

### MacOS or Linux

Download [OpenOCD v0.11.0-1](https://github.com/xpack-dev-tools/openocd-xpack/releases/tag/v0.11.0-1) to the working directory `$EGOS`.

```shell
> cd $EGOS
> tar -zxvf xpack-openocd-0.11.0-1-xxx-xxx.tar.gz
> export PATH=$PATH:$EGOS/xpack-openocd-0.11.0-1-xxx-xxx/bin
> cd $EGOS/egos-2000
> make program
-------- Program the on-board ROM --------
cd tools/openocd; time openocd -f 7series.txt
......
Info : sector 188 took 223 ms
Info : sector 189 took 223 ms
Info : sector 190 took 229 ms
Info : sector 191 took 243 ms  # It will pause at this point for a while
Info : Found flash device 'micron n25q128' (ID 0x0018ba20)

real    2m51.019s
user    0m11.540s
sys     0m37.338s

```

4. In `$EGOS`, type `make program`; If fails, restart your machine and retry
5. Wait for about 3 minutes until the program finishes

### Windows or Linux
Install Vivado Lab Edition which can be downloaded [here](https://www.xilinx.com/support/download.html).
You may need to register a Xilinx account, but the software is free.

1. Connect the Arty FPGA board to your computer with the USB cable
2. Open Vivado Lab Edition
3. Click "Open Hardware Manager"
4. Click "Open target" and "Auto Connect"; the Arty board should appear in the "Hardware" window
5. In the "Hardware" window, right click `xc7a35t` and click "Add Configuration Memory Device"
6. Choose memory device "mt25ql128-spi-x1_x2_x4" and click "Program Configuration Memory Device"
7. In the "Configuration file" field, choose the `bootROM.mcs` file compiled in step 2
8. Click "OK" and wait for the program to finish

In **6**, new versions of Arty may use "s25fl128sxxxxxx0" as memory device. 
If you choose the wrong one, step8 will tell you.

In **4**, if the Arty board is not detected, try to reinstall the USB cable drivers following [these instructions](https://support.xilinx.com/s/article/59128?language=en_US). If it still doesn't work, it may be an issue with Vivado and please [contact Xilinx](https://support.xilinx.com/s/topic/0TO2E000000YKXgWAO/programmable-logic-io-bootconfiguration?language=en_US) or try the openocd method described above.

![This is an image](https://dolobyte.net/print/vivado.png)


## Step4: Connect with egos-2000

1. Press the `program` red button on the left-top corner of the Arty board
2. For Linux users, type in your shell
```shell
> sudo chmod 666 /dev/ttyUSB1
> screen /dev/ttyUSB1 115200
```
3. For Mac users, use the same commands but check your `/dev` directory for the  device name (e.g., `/dev/tty.usbserial-xxxxxx`)
4. For Windows users, use software like `PuTTY` to connect with the serial port (e.g., COM6) and use baud rate 115200
5. Press the `reset` red button on the right-top corner to restart egos-2000