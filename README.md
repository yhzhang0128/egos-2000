# A minimal operating system for a real RISC-V board

Within **2.7k** lines of code, our teaching OS implements SD card and TTY driver, interrupt handling, address translation, process creation and scheduling, system calls, file system, shell and 4 shell commands.

![This is an image](https://dolobyte.net/print/egos-riscv.jpg)

The earth and grass operating system (EGOS) is our teaching OS at Cornell. It has three layers: 

* the **earth layer** provides *hardware-specific* abstractions
    * tty and disk device interfaces
    * cpu interrupt and memory management interfaces
* the **grass layer** provides *hardware-independent* abstractions
    * processes and system calls
    * inter-process communication
* the **application layer** on top of the operating system provides
    * file system
    * shell and shell commands

EGOS is minimal and very suitable for teaching an undergraduate operating system course.

```shell
> cloc egos-riscv
      52 text files.
      48 unique files.                              
       8 files ignored.

github.com/AlDanial/cloc v 1.92  T=0.03 s (1675.5 files/s, 135576.0 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C                               27            512            395           2021
C/C++ Header                    15             99            110            406
Markdown                         2             35              0            145
Assembly                         3              3             21             69
make                             1             12              0             56
-------------------------------------------------------------------------------
SUM:                            48            661            526           2697
-------------------------------------------------------------------------------
```

## Hardware requirements
* an Artix-7 35T [Arty FPGA Development Board](https://digilent.com/shop/arty-a7-artix-7-fpga-development-board/)
* a microUSB cable (e.g., [microUSB-to-USB](https://www.amazon.com/CableCreation-Charging-Shielded-Charger-Compatible/dp/B07CKXQ9NB?ref_=ast_sto_dp&th=1&psc=1) or [microUSB-to-USB-C](https://www.amazon.com/dp/B0744BKDRD?psc=1&ref=ppx_yo2_dt_b_product_details))
* [optional] an [SDHC microSD card](https://www.amazon.com/dp/B073K14CVB?ref=ppx_yo2_dt_b_product_details&th=1), a [microSD Pmod](https://digilent.com/reference/pmod/pmodmicrosd/start?redirect=1) and a [USB microSD reader](https://www.amazon.com/dp/B07G5JV2B5?psc=1&ref=ppx_yo2_dt_b_product_details)

## Software requirements
* SiFive [freedom riscv-gcc compiler toolchain](https://github.com/sifive/freedom-tools/releases/tag/v2020.04.0-Toolchain.Only)
* [Vivado lab solutions](https://www.xilinx.com/support/download.html) or any edition with the hardware manager
* a software to connect with ttyUSB
    * e.g., [screen](https://linux.die.net/man/1/screen) for Linux/Mac or [PuTTY](https://www.putty.org/) for Windows
* [optional] a tool to program a disk image file to the microSD card 
    * e.g., [balena Etcher](https://www.balena.io/etcher/)

## Tutorial and documentation

For compiling and running egos-riscv, you can read [USAGES.md](USAGES.md). 
The [documentation](../../../documentation) further introduces the teaching plans, architecture and development history of egos-riscv.

For any questions, please contact [Yunhao Zhang](https://dolobyte.net/) or [Robbert van Renesse](https://www.cs.cornell.edu/home/rvr/).
