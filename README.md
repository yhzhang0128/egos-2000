## Vision

This project's vision is to help **every** college student read **all** the code of an operating system.

With only **2000** lines of code, egos-2000 implements every component of an operating system for education. 
It can run on RISC-V boards and the QEMU software emulator.

![Fail to load an image of egos-2000.](references/screenshots/egos-2000.jpg)

```shell
# The cloc utility is used to count the lines of code (LOC).
# The command below counts the LOC of everything excluding text documents.
> cloc egos-2000 --exclude-ext=md,txt
...
github.com/AlDanial/cloc v 1.94  T=0.05 s (949.3 files/s, 62349.4 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C                               37            508            657           1577
C/C++ Header                    10             69            105            283
Assembly                         3             10             27             76
make                             1             14              3             64
-------------------------------------------------------------------------------
SUM:                            51            601            792           2000 (exactly 2000!)
-------------------------------------------------------------------------------
```

## Earth and Grass Operating System

The **egos** part of egos-2000 is named after its three-layer architecture.

* The **earth layer** implements hardware-specific abstractions.
    * tty and disk device interface
    * timer, exception and memory management interface
* The **grass layer** implements hardware-independent abstractions.
    * process control block and system call interface
* The **application layer** implements file system, shell and user commands.

The definitions of `struct earth` and `struct grass` in [this header file](library/egos.h) specify the layer interface.

For compiling and running egos-2000, please read [this document](references/USAGES.md).
The [RISC-V instruction set manual](references/riscv-privileged-v1.10.pdf) and [SiFive FE310 processor manual](references/sifive-fe310-v19p04.pdf) introduce the privileged ISA and memory map.
[This document](references/README.md) further introduces the teaching plans, software architecture and development history.

## Acknowledgements

Many thanks to Meta for a [Meta fellowship](https://research.facebook.com/fellows/zhang-yunhao/).
Many thanks to [Robbert van Renesse](https://www.cs.cornell.edu/home/rvr/), [Lorenzo Alvisi](https://www.cs.cornell.edu/lorenzo/), [Shan Lu](https://people.cs.uchicago.edu/~shanlu/), [Hakim Weatherspoon](https://www.cs.cornell.edu/~hweather/) and [Christopher Batten](https://www.csl.cornell.edu/~cbatten/) for their support.
Many thanks to all the [CS5411/4411](https://www.cs.cornell.edu/courses/cs4411/2022fa/schedule/) students at Cornell over the years for helping improve this course.
Many thanks to [Cheng Tan](https://naizhengtan.github.io/) for providing valuable feedback and using egos-2000 in his [CS6640 at Northeastern](https://naizhengtan.github.io/23fall/).
Many thanks to Brandon Fusi for [porting to the Allwinner's D1 chip](https://github.com/cheofusi/egos-2000-d1) using Sipeed's Lichee RV64 compute module.

For any questions, please contact [Yunhao Zhang](https://dolobyte.net/).