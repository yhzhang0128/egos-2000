📢 The first draft of the [EGOS book](https://egos.fun/book/overview.html) has been released which contains 9 course projects.

## Vision

This project's vision is to help **every** student read **all** the code of a teaching operating system.

With only **2000** lines of code, egos-2000 implements every component of a teaching operating system that runs on both QEMU and RISC-V boards.
The [EGOS book](https://egos.fun/book/overview.html) contains 9 course projects based on egos-2000.

![Fail to load an image of egos-2000.](tools/screenshots/egos-2000.jpg)

```shell
# The cloc utility is used to count the lines of code.
> cloc egos-2000 --exclude-lang=Markdown,Text,TOML,make   # excluding text files
      71 text files.
      49 unique files.
      60 files ignored.

github.com/AlDanial/cloc v 2.04  T=0.02 s (2506.4 files/s, 194031.9 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C                               29            424            578           1595
C/C++ Header                     9             62            100            260
Assembly                         3             15             46             94
-------------------------------------------------------------------------------
SUM:                            41            501            724           1949
-------------------------------------------------------------------------------
```

## Earth and Grass Operating System

The **egos** part of egos-2000 is named after its three-layer architecture.

* The **earth layer** implements hardware-specific abstractions.
    * tty and disk device interface
    * timer and memory management interface
* The **grass layer** implements hardware-independent abstractions.
    * process control block and system call interface
* The **application layer** implements file system, shell and user commands.

The definitions of `struct earth` and `struct grass` in header file [egos.h](library/egos.h) specify the layer interface.
Please read [USAGES.md](USAGES.md) for running egos-2000 and
the [instruction set manuals](https://github.com/riscv/riscv-isa-manual/releases) for the RISC-V privileged ISA.

## Acknowledgements

Many thanks to Meta for a [Facebook fellowship](https://research.facebook.com/blog/2021/4/announcing-the-recipients-of-the-2021-facebook-fellowship-awards/).
Many thanks to [Robbert van Renesse](https://www.cs.cornell.edu/home/rvr/), [Lorenzo Alvisi](https://www.cs.cornell.edu/lorenzo/), [Shan Lu](https://people.cs.uchicago.edu/~shanlu/), [Hakim Weatherspoon](https://www.cs.cornell.edu/~hweather/) and [Christopher Batten](https://www.csl.cornell.edu/~cbatten/) for their support.
Many thanks to [Cheng Tan](https://naizhengtan.github.io/) and [Yu-Ju Huang](https://yuju-huang.github.io/) for providing valuable feedback to the EGOS book and using egos-2000 in [Northeastern CS4973/6640](https://naizhengtan.github.io/25spring/) and [Cornell CS4411/5411](https://www.cs.cornell.edu/courses/cs4411/2025sp/).
Many thanks to [Haobin Ni](https://haobin.cx/) and [Hongbo Zhang](https://www.cs.cornell.edu/~hongbo/) for [porting egos-2000 to mriscv](https://github.com/0x486F626F/mriscv/tree/egos), a simple processor written in SystemVerilog.
Many thanks to [Brandon Fusi](https://www.linkedin.com/in/brandon-cheo-fusi-b94b1a171/) for [porting egos-2000 to Allwinner D1](https://github.com/cheofusi/egos-2000-d1) and Sipeed's [Lichee RV64 Nezha compute module](https://wiki.sipeed.com/hardware/en/lichee/RV/RV.html).

For any questions, please contact [Yunhao Zhang](https://dolobyte.net/).
