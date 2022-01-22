# A minimal operating system on a $129 RISC-V board

![This is an image](https://dolobyte.net/print/egos-riscv.jpg)

## Lines of code
```shell
> make loc
cloc . --exclude-dir=install
      31 text files.
      31 unique files.                              
       5 files ignored.

github.com/AlDanial/cloc v 1.82  T=0.02 s (1657.9 files/s, 91367.4 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C                               13            228            137            757
C/C++ Header                    11             64              5            205
make                             1             11              0             43
Assembly                         2              6             14             18
-------------------------------------------------------------------------------
SUM:                            27            309            156           1023
-------------------------------------------------------------------------------

```
# Earth and grass operating system (EGOS)

EGOS is our teaching operating system at Cornell for CS4411. It has 3 layers: 
* the earth layer provides **hardware-specific** abstractions
    * tty and disk device driver interface
    * cpu interrupt and memory management interface
* the grass layer provides **hardware-independent** abstractions
    * processes
    * system calls
    * inter-process communication
* the application layer on top of the operating system provides
    * file system
    * shell
    * common shell commands

## Hardware requirements
* an Artix-7 35T [Arty FPGA Development Board](https://digilent.com/shop/arty-a7-artix-7-fpga-development-board/)
* a microUSB cable, such as [microUSB-to-USB](https://www.amazon.com/CableCreation-Charging-Shielded-Charger-Compatible/dp/B07CKXQ9NB?ref_=ast_sto_dp&th=1&psc=1) or [microUSB-to-USB-C](https://www.amazon.com/dp/B0744BKDRD?psc=1&ref=ppx_yo2_dt_b_product_details)
* an **SDHC** microSD card, a [microSD Pmod](https://digilent.com/reference/pmod/pmodmicrosd/start?redirect=1) and a [USB microSD card reader](https://www.amazon.com/dp/B07G5JV2B5?psc=1&ref=ppx_yo2_dt_b_product_details)

## Software requirements
* Sifive [freedom toolchain](https://github.com/sifive/freedom-tools/releases/tag/v2020.04.0-Toolchain.Only)
* Vivado [lab solutions](https://www.xilinx.com/support/download.html) or any edition with the hardware manager
* a tool to program a disk image file to the microSD card 
    * such as [balena Etcher](https://www.balena.io/etcher/)
* a tool to connect with ttyUSB
    * such as [PuTTY](https://www.putty.org/) for Windows or [screen](https://linux.die.net/man/1/screen) for Linux and Mac

## Compile, install and further information

**TODO**: a Youtube video tutorial

Please read the documents in the [EGOS documentation repository](). 
For any questions, please contact [Yunhao Zhang](https://dolobyte.net/) or [Robbert van Renesse](https://www.cs.cornell.edu/home/rvr/).
