# A minimal operating system on a $99 RISC-V board

![This is an image](https://dolobyte.net/print/egos-riscv.jpg)

```shell
> make loc
cloc . --exclude-dir=install
      31 text files.
      27 unique files.                              
       5 files ignored.

github.com/AlDanial/cloc v 1.92  T=0.02 s (1120.1 files/s, 61602.9 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C                               13            228            137            757
C/C++ Header                    11             64              2            205
make                             1             11              0             43
Assembly                         2              6             14             18
-------------------------------------------------------------------------------
SUM:                            27            309            153           1023
-------------------------------------------------------------------------------
```
# EGOS: the earth and grass operating system

EGOS is our teaching operating system at Cornell. As the name suggests, it has two layers: 
* the earth layer provides **hardware-specific** abstractions
    * tty and disk device driver interface
    * cpu interrupt and memory management interface
* the grass layer provides **hardware-independent** abstractions
    * processes
    * system calls
    * inter-process communication

For more information, please read the documents in the [EGOS documentation Github repo](). 
For any questions, please contact [Yunhao Zhang](mailto:yz2327@cornell.edu) or [Robbert van Renesse](mailto:rvr@cs.cornell.edu).
