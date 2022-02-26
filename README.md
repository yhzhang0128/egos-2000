# A minimal operating system for a real RISC-V board

![This is an image](https://dolobyte.net/print/egos-riscv.jpg)

The earth and grass operating system (EGOS) is our teaching OS at Cornell. It has three layers: 

* the earth layer provides **hardware-specific** abstractions
    * tty and disk device interfaces
    * cpu interrupt and memory management interfaces
* the grass layer provides **hardware-independent** abstractions
    * processes and system calls
    * inter-process communication
* the application layer on top of the operating system provides
    * file system
    * shell and common shell commands

EGOS is minimal and very suitable for teaching an undergraduate operating system course.

```shell
> cloc .
      41 text files.
      33 unique files.                              
      16 files ignored.

github.com/AlDanial/cloc v 1.92  T=0.03 s (1089.7 files/s, 70829.1 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C                               16            275            186            966
C/C++ Header                    11             76             17            276
Markdown                         2             35              0            145
Assembly                         3             10             21             70
make                             1             12              0             56
-------------------------------------------------------------------------------
SUM:                            33            408            224           1513
-------------------------------------------------------------------------------
```

## Hardware requirements
* an Artix-7 35T [Arty FPGA Development Board](https://digilent.com/shop/arty-a7-artix-7-fpga-development-board/)
* a microUSB cable (e.g., [microUSB-to-USB](https://www.amazon.com/CableCreation-Charging-Shielded-Charger-Compatible/dp/B07CKXQ9NB?ref_=ast_sto_dp&th=1&psc=1) or [microUSB-to-USB-C](https://www.amazon.com/dp/B0744BKDRD?psc=1&ref=ppx_yo2_dt_b_product_details))
* an [SDHC microSD card](https://www.amazon.com/dp/B073K14CVB?ref=ppx_yo2_dt_b_product_details&th=1), a [microSD Pmod](https://digilent.com/reference/pmod/pmodmicrosd/start?redirect=1) and a [USB microSD reader](https://www.amazon.com/dp/B07G5JV2B5?psc=1&ref=ppx_yo2_dt_b_product_details)

## Software requirements
* SiFive [freedom riscv-gcc compiler toolchain](https://github.com/sifive/freedom-tools/releases/tag/v2020.04.0-Toolchain.Only)
* [Vivado lab solutions](https://www.xilinx.com/support/download.html) or any edition with the hardware manager
* a tool to program a disk image file to the microSD card 
    * e.g., [balena Etcher](https://www.balena.io/etcher/)
* a tool to connect with ttyUSB
    * e.g., [screen](https://linux.die.net/man/1/screen) for Linux/Mac or [PuTTY](https://www.putty.org/) for Windows

## Tutorial and documentation

For compiling and running egos-riscv, you can read [USAGES.md](USAGES.md) or watch our video tutorial ([Youtube](), [Bilibili]()). 
The [documentation](../../../documentation) further introduces the teaching plans, architecture and development history of egos-riscv.

For any questions, please contact [Yunhao Zhang](https://dolobyte.net/) or [Robbert van Renesse](https://www.cs.cornell.edu/home/rvr/).
