## Vision

This project's vision is to help **every** college student read **all** the code of an operating system.

With only **2000** lines of code, egos-2000 implements every component of an operating system for education. 
It can run on a RISC-V board and the QEMU software emulator.

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
C                               37            509            668           1576
C/C++ Header                    10             68            105            286
Assembly                         4              6             31             72
make                             1             13              1             66
-------------------------------------------------------------------------------
SUM:                            52            596            805           2000 (exactly 2000!)
-------------------------------------------------------------------------------
```

## Earth and Grass Operating System

The **egos** part of egos-2000 is named after its 3-layer architecture.

* The **earth layer** implements hardware-specific abstractions.
    * tty and disk device interfaces
    * interrupt and memory management interfaces
* The **grass layer** implements hardware-independent abstractions.
    * processes, system calls and inter-process communications
* The **application layer** implements file system, shell and user commands.

The definitions of `struct earth` and `struct grass` in [this header file](library/egos.h) specify the layer interfaces.

Developed for [CS4411 at Cornell](https://www.cs.cornell.edu/courses/cs4411/2022fa/schedule/),
egos-2000 also has a [special version](https://github.com/yhzhang0128/egos-2000/tree/ece4750) running on the RISC-V processor from [ECE4750 at Cornell](https://github.com/cornell-ece4750).
The goal is to make OS courses more connected with computer architecture courses.

### Usages and Documentation

For compiling and running egos-2000, please read [this document](references/USAGES.md).
The [RISC-V instruction set manual](references/riscv-privileged-v1.10.pdf) and [SiFive FE310 processor manual](references/sifive-fe310-v19p04.pdf) introduce the privileged ISA and memory map.
[This document](references/README.md) further introduces the teaching plans, software architecture and development history.

For any questions, please contact [Yunhao Zhang](https://dolobyte.net/).

## Acknowledgements

Many thanks to [Robbert van Renesse](https://www.cs.cornell.edu/home/rvr/), [Lorenzo Alvisi](https://www.cs.cornell.edu/lorenzo/), [Shan Lu](https://people.cs.uchicago.edu/~shanlu/), [Hakim Weatherspoon](https://www.cs.cornell.edu/~hweather/) and [Christopher Batten](https://www.csl.cornell.edu/~cbatten/) for their support.
Many thanks to Meta for a [Meta fellowship](https://research.facebook.com/fellows/zhang-yunhao/).
Many thanks to all CS5411/4411 students at Cornell over the years for helping improve this course.
