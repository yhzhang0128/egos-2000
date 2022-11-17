With only **2000** lines of code, **egos-2000** implements every component of an operating system for education.

**egos-2000** can run on a RISC-V board and the QEMU software emulator.
The vision of this project is to help **every** college student read **all** the code of an operating system within **one** semester.

![This is an image](references/screenshots/egos-2000.jpg)
Note: [**cloc**](https://github.com/AlDanial/cloc) is used to count the lines of code above. Try `cloc egos-2000 --exclude-ext=md` which excludes the markdown documents in the count.

## Earth and Grass Operating System 2000

We use **egos-2000** as our teaching operating system at Cornell. It adopts a 3-layer architecture.

* The **earth layer** implements hardware-specific abstractions.
    * tty and disk device interfaces
    * interrupt and memory management interfaces
* The **grass layer** implements hardware-independent abstractions.
    * processes, system calls and inter-process communications
* The **application layer** implements file system, shell and user commands.

The definitions of `struct earth` and `struct grass` in [**egos.h**](library/egos.h) specify the interfaces of these layers.

### Hardware Requirements
* an Artix-7 35T [Arty FPGA development board](https://www.xilinx.com/products/boards-and-kits/arty.html)
* a microUSB cable (e.g., [microUSB-to-USB-C](https://www.amazon.com/dp/B0744BKDRD?psc=1&ref=ppx_yo2_dt_b_product_details))
* [optional] a [microSD Pmod](https://digilent.com/reference/pmod/pmodmicrosd/start?redirect=1), a [microSD reader](https://www.amazon.com/dp/B07G5JV2B5?psc=1&ref=ppx_yo2_dt_b_product_details) and a microSD card (e.g., [Sandisk](https://www.amazon.com/dp/B073K14CVB?ref=ppx_yo2_dt_b_product_details&th=1))

Don't worry if the hardware is not available to you. You can also use the software emulator QEMU.

### Usages and Documentation

For compiling and running egos-2000, please read [USAGES.md](references/USAGES.md).

The [RISC-V instruction set manual](references/riscv-privileged-v1.10.pdf) introduces the privileged ISA.
The [SiFive FE310 manual](references/sifive-fe310-v19p04.pdf) introduces the memory map, especially the GPIO, UART and SPI bus controllers.
[This document](references/README.md) further introduces the teaching plans, architecture and development history of egos-2000.

For any questions, please contact [Yunhao Zhang](https://dolobyte.net/).

## Acknowledgements

Many thanks to [Robbert van Renesse](https://www.cs.cornell.edu/home/rvr/) and [Lorenzo Alvisi](https://www.cs.cornell.edu/lorenzo/) for their support.
Many thanks to [Meta](https://about.facebook.com/meta/) for supporting me with a [fellowship](https://research.facebook.com/fellows/zhang-yunhao/).
Many thanks to all CS4411 students at Cornell over the years for helping improve this course.
