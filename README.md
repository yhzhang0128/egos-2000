## Vision Statement

Our vision is to help **every** college student read **all** the code of an operating system within **one** semester.

With only **2000** lines of code, **egos-2000** implements every component of an operating system for education. 
It can run on a RISC-V board and also the QEMU software emulator.

![This is an image](references/screenshots/egos-2000.jpg)
Note: [**cloc**](https://github.com/AlDanial/cloc) was used to count the lines of code.
Try `cloc egos-2000 --exclude-ext=md` which excludes the markdown documents when counting code lines.

## Earth and Grass Operating System

We use **egos-classic** and **egos-2000** as our teaching operating system at Cornell. They both adopt a 3-layer architecture.

* The **earth layer** implements hardware-specific abstractions.
    * tty and disk device interfaces
    * interrupt and memory management interfaces
* The **grass layer** implements hardware-independent abstractions.
    * processes, system calls and inter-process communications
* The **application layer** implements file system, shell and user commands.

The definitions of `struct earth` and `struct grass` in [**egos.h**](library/egos.h) specify the interfaces of these layers.

### Usages and Documentation

For compiling and running egos-2000, please read [USAGES.md](references/USAGES.md).

The [RISC-V instruction set manual](references/riscv-privileged-v1.10.pdf) introduces the privileged ISA.
The [SiFive FE310 manual](references/sifive-fe310-v19p04.pdf) introduces the memory map, especially the GPIO, UART and SPI bus controllers.
[This document](references/README.md) further introduces the teaching plans, architecture and development history of egos-2000.

For any questions, please contact [Yunhao Zhang](https://dolobyte.net/).

## Acknowledgements

Many thanks to [Robbert van Renesse](https://www.cs.cornell.edu/home/rvr/), [Lorenzo Alvisi](https://www.cs.cornell.edu/lorenzo/), [Shan Lu](https://people.cs.uchicago.edu/~shanlu/) and [Hakim Weatherspoon](https://www.cs.cornell.edu/~hweather/) for supporting this project.
Many thanks to Meta for a [Meta fellowship](https://research.facebook.com/fellows/zhang-yunhao/).
Many thanks to all CS5411/4411 students at Cornell over the years for helping improve this course.
