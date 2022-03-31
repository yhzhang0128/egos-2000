# A minimal operating system on a real RISC-V board

With only **2.1K** lines of code, our teaching OS implements microSD driver, tty driver, interrupt handling, address translation, process scheduling and messaging, system call, file system, shell and 7 shell commands.

![This is an image](https://dolobyte.net/print/egos-riscv.jpg)

The earth and grass operating system (EGOS) is our teaching OS at Cornell. It has three layers: 

* The **earth layer** implements hardware-specific abstractions.
    * tty and disk device interfaces
    * cpu interrupt and memory management interfaces
* The **grass layer** implements hardware-independent abstractions.
    * processes and system calls
    * inter-process communication
* The **application layer** provides file system, shell and shell commands.

This RISC-V version of EGOS is minimal in order to give students a complete picture.

```shell
# Count lines of code excluding README.md and USAGES.md (LOC = 2099)
> cloc egos-riscv --exclude-ext=md  
      52 text files.
      52 unique files.
       7 files ignored.

github.com/AlDanial/cloc v 1.82  T=0.03 s (1799.3 files/s, 124657.3 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C                               30            448            414           1614
C/C++ Header                    13             90            107            364
Assembly                         2              4             14             67
make                             1             11              0             54
-------------------------------------------------------------------------------
SUM:                            46            553            535           2099
-------------------------------------------------------------------------------
```

## Hardware requirements
* an Artix-7 35T [Arty FPGA development board](https://www.xilinx.com/products/boards-and-kits/arty.html)
* a microUSB cable (e.g., [microUSB-to-USB](https://www.amazon.com/CableCreation-Charging-Shielded-Charger-Compatible/dp/B07CKXQ9NB?ref_=ast_sto_dp&th=1&psc=1) or [microUSB-to-USB-C](https://www.amazon.com/dp/B0744BKDRD?psc=1&ref=ppx_yo2_dt_b_product_details))
* [optional] an [SDHC microSD card](https://www.amazon.com/dp/B073K14CVB?ref=ppx_yo2_dt_b_product_details&th=1), a [microSD Pmod](https://digilent.com/reference/pmod/pmodmicrosd/start?redirect=1) and a [USB microSD reader](https://www.amazon.com/dp/B07G5JV2B5?psc=1&ref=ppx_yo2_dt_b_product_details)

## Software requirements
* SiFive [freedom riscv-gcc compiler](https://github.com/sifive/freedom-tools/releases/tag/v2020.04.0-Toolchain.Only)
* [Vivado lab solutions](https://www.xilinx.com/support/download.html) or any edition with the hardware manager
* a software to connect with ttyUSB
    * e.g., [screen](https://linux.die.net/man/1/screen) for Linux/Mac or [PuTTY](https://www.putty.org/) for Windows
* [optional] a tool to program a disk image file to the microSD card 
    * e.g., [balena Etcher](https://www.balena.io/etcher/) for all platforms

## Usages and documentation

For compiling and running egos-riscv, please read [USAGES.md](USAGES.md). 
The [documentation](../../../documentation) further introduces the teaching plans, architecture and development history of egos-riscv.

For any questions, please contact [Yunhao Zhang](https://dolobyte.net/) or [Robbert van Renesse](https://www.cs.cornell.edu/home/rvr/).
