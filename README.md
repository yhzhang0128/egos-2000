# A minimal operating system on a small RISC-V board

With only **2K** lines of code, **egos-2000** implements boot loader, microSD driver, tty driver, memory paging, address translation, interrupt handling, process scheduling and messaging, system call, file system, shell, 7 user commands and the `mkfs/mkrom` tools.
It runs on a **$129** small development board.

![This is an image](https://dolobyte.net/print/egos-riscv.jpg)

```shell
# Count lines of code excluding references and README.md
> cloc egos-2000 --exclude-ext=md,pdf
github.com/AlDanial/cloc v 1.92  T=0.03 s (1668.0 files/s, 105414.7 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C                               35            453            486           1566
C/C++ Header                    11             75            105            312
Assembly                         3              6             24             68
make                             1             11              0             54
-------------------------------------------------------------------------------
SUM:                            50            545            615           2000 << exactly 2000!
-------------------------------------------------------------------------------
```

## Earth and Grass Operating System 2000

**egos-2000** and [**egos**]() are the teaching operating systems we use at Cornell. They have the same architecture.

* The **earth layer** implements hardware-specific abstractions.
    * tty and disk device interfaces
    * cpu interrupt and memory management interfaces
* The **grass layer** implements hardware-independent abstractions.
    * processes, system calls and inter-process communication
* The **application layer** implements file system, shell and user commands.

The definitions of `struct earth` and `struct grass` in [egos.h](library/egos.h) specify the interfaces between different layers.

## Hardware Requirements
* an Artix-7 35T [Arty FPGA development board](https://www.xilinx.com/products/boards-and-kits/arty.html)
* a microUSB cable (e.g., [microUSB-to-USB](https://www.amazon.com/CableCreation-Charging-Shielded-Charger-Compatible/dp/B07CKXQ9NB?ref_=ast_sto_dp&th=1&psc=1) or [microUSB-to-USB-C](https://www.amazon.com/dp/B0744BKDRD?psc=1&ref=ppx_yo2_dt_b_product_details))
* [optional] a [microSD Pmod](https://digilent.com/reference/pmod/pmodmicrosd/start?redirect=1), a [microSD reader](https://www.amazon.com/dp/B07G5JV2B5?psc=1&ref=ppx_yo2_dt_b_product_details) and a microSD card (e.g., [Sandisk](https://www.amazon.com/dp/B073K14CVB?ref=ppx_yo2_dt_b_product_details&th=1), [Samsung](https://www.amazon.com/dp/B09B1F9L52?ref=ppx_yo2_dt_b_product_details&th=1) or [PNY](https://www.amazon.com/dp/B08RG87JN5?ref=ppx_yo2_dt_b_product_details&th=1))

## Software Requirements
* SiFive [freedom riscv-gcc compiler](https://github.com/sifive/freedom-tools/releases/tag/v2020.04.0-Toolchain.Only)
* [Vivado lab solutions](https://www.xilinx.com/support/download.html) or any edition with the hardware manager
* a software to connect with ttyUSB (e.g., [screen](https://linux.die.net/man/1/screen) for Linux/Mac or [PuTTY](https://www.putty.org/) for Windows)
* [optional] a software to program a disk image file to the microSD card (e.g., [dd](https://linux.die.net/man/1/dd) or [balena Etcher](https://www.balena.io/etcher/)) 

## Usages and Documentation

For compiling and running egos-2000, please read [USAGES.md](references/USAGES.md). 
[This document](references/README.md) further introduces the teaching plans, architecture and development history of egos-2000.

The [RISC-V instruction set manual](references/riscv-privileged-v1.10.pdf) introduces the privileged registers used by egos-2000.
The [SiFive FE310 manual](references/sifive-fe310-v19p04.pdf) introduces the processor used by egos-2000, especially the GPIO, UART and SPI bus controllers.

For any questions, please contact [Yunhao Zhang](https://dolobyte.net/).
Many thanks to [Robbert van Renesse](https://www.cs.cornell.edu/home/rvr/) and [Lorenzo Alvisi](https://www.cs.cornell.edu/lorenzo/) for their support.
This project is also supported by a [Facebook fellowship](https://research.facebook.com/fellows/zhang-yunhao/).
