## Teaching plan

|    | Description                       | Concepts to teach                                           | Platform  |
|----|-----------------------------------|-------------------------------------------------------------|-----------|
| P0 | Queue data structure in C         | memory, pointers, user-level ISA                            | POSIX     |
| P1 | Non-preemptive Multi-threading    | context switch, multi-threading, synchronization            | QEMU/Arty |
| P2 | Multi-level Feedback Queue        | timer, interrupt handling, scheduling                       | QEMU/Arty |
| P3 | Memory Protection and System Call | exception, control and status register, privilege level     | QEMU/Arty |
| P4 | File System                       | inode layer, directory layer                                | QEMU/Arty |
| P5 | Page Table Translation            | RISC-V Sv32 translation                                     | QEMU      |
| P6 | Multi-core and Atomic Instruction | spinlock, synchronization, atomic instruction               | QEMU      |
| P7 | SD Card Driver                    | I/O device, bus controller, device driver                   | QEMU/Arty |

## Architecture

**Earth layer (hardware specific)**
* `earth/dev_disk`: ROM and SD card (touched by P5)
* `earth/dev_page`: memory paging
* `earth/dev_tty`: keyboard input and tty output
* `earth/cpu_intr`: interrupt and exception handling (touched by P3)
* `earth/cpu_mmu`: memory paging and address translation (touched by P3, P6)

**Grass layer (hardware independent)**
* `grass/syscall`: system call interfaces to user applications
* `grass/process`: data structures for managing processes (touched by P1)
* `grass/kernel`: preemptive scheduling and inter-process communication (touched by P2)

**Application layer**
* `app/system/sys_proc`: allocation and free of user processes
* `app/system/sys_file`: the inode layer over the SD card (touched by P4)
* `app/system/sys_dir`: the directory layer over the inode layer (touched by P4)
* `app/system/sys_shell`: interactive user interface
* shell commands: `pwd`, `cd`, `ls`, `cat`, `echo`, `clock`, `killall`

Every layer has dedicated memory regions as described below (and in `library/egos.h`).
The complete memory layout is described in Chapter 4 of the [FE310 manual](sifive-fe310-v19p04.pdf).

**Selected memory regions**

| Base        | Top         | Attributes | Description       | Notes                                                      |
|-------------|-------------|------------|-------------------|------------------------------------------------------------|
| 0x0800_0000 | 0x0800_1FFF | RWX A      | ITIM, 10KB        | Earth layer bss, data and heap                             |
| 0x0800_2000 | 0x0800_4FFF | RWX A      | ITIM, 10KB        | Grass layer code, bss, data and heap                       |
| 0x0800_5000 | 0x0800_7FFF | RWX A      | ITIM, 12KB        | App layer code, bss, data and heap                         |
| ......      | ......      | ......     | ......            |                                                            |
| 0x2000_0000 | 0x203F_FFFF | R XC       | Flash ROM, 4MB    | FPGA binary of the FE310 RISC-V processor                  |
| 0x2040_0000 | 0x207F_FFFF | R XC       | Flash ROM, 4MB    | Earth layer binary                                         |
| 0x2080_0000 | 0x20BF_FFFF | R XC       | Flash ROM, 4MB    | Disk image (disk.img produced by mkrom)                    |
| ......      | ......      | ......     | ......            |                                                            |
| 0x8000_0000 | 0x8000_1FFF | RW- A      | DTIM, 8KB         | App layer stack                                            |
| 0x8000_2000 | 0x8000_3FFF | RW- A      | DTIM, 8KB         | Earth layer and grass layer stack                          |
| 0x8000_4000 | 0x8001_FFFF | RW- A      | DTIM, 112KB       | MMU cache of physical frames for suspended grass processes |

On the Arty board, the first 1MB of the microSD card is used as 256 physical frames by the MMU for paging (see `earth/dev_page.c`).

## Software development history

**Iteration #9**
* [2024.04] Run egos-2000 on the [mriscv processor](https://github.com/0x486F626F/mriscv/tree/egos) in System Verilog
* [2024.04] Improve type usage, especially the use of unsigned types
* [2024.05] Improve the trap entry assembly code: entering kernel stack right after exceptions
* [2024.05] Switch to `sifive_u` in QEMU and run the SD driver code on emulated `sifive_u` machine
* [2024.06] Improve system call implementation: making `SYS_SEND` and `SYS_RECV` asynchronous
* [2024.06] Enable multi-core in QEMU and create a new project for spinlock and synchronization

**Iteration #8**
* [2023.05] Add `apps/user/ult.c`, a project on non-preemptive threading
* [2023.08] Create the `ece4750` branch and run egos-2000 on [this teaching processor](https://github.com/cornell-ece4750/)
* [2023.09] Add support for the [official GNU C compiler](https://github.com/riscv-collab/riscv-gnu-toolchain)
* [2023.09] Add support for the Arty S7-50 and A7-100t boards
* [2023.09] Simplify page table translation by using virutal addresses in the earth layer (`mstatus.MPRV`)

**Iteration #7**
* [2022.10] Add support to the QEMU emulator
* [2022.10] Enable the supervisor mode in the QEMU emulator
* [2022.10] With the modified QEMU, add page table translation in `earth/cpu_mmu.c`
* [2022.10] Use egos-2000 in CS4411/5411 at Cornell University and obtain [5-star rating](https://www.ratemyprofessors.com/professor/2651034)

**Iteration #6**
* [2022.04] Add a simple boot loader `earth/earth.S`; Remove dependency on the Metal library
* [2022.04] Add `_write()` and `_sbrk()` in `library/libc`; Remove the Metal library entirely
* [2022.04] Enrich `struct grass` in order to improve clarity of the architecture
* [2022.04] Experiment with Physical Memory Protection (PMP) and switching privilege level (machine <-> user)

**Iteration #5**
* [2022.03] Implement system calls and 4 shell commands: `pwd`, `ls`, `cat` and `echo`
* [2022.03] Add support of background shell commands and `killall`
* [2022.03] Add servers in `library` and cleanup access to the file system
* [2022.03] Cleanup the code controlling UART and SPI; Remove dependency on the Metal library

**Iteration #4**
* [2022.02] Read Chapter 1, 2 and 3 of [RISC-V manual](riscv-privileged-v1.10.pdf)
* [2022.02] Read Chapter 8, 9 and 10 of [FE310 manual](sifive-fe310-v19p04.pdf)
* [2022.02] Implement timer reset, preemptive scheduling and inter-process communication
* [2022.02] Implement the kernel processes: GPID_PROCESS, GPID_FILE, GPID_DIR, GPID_SHELL

**Iteration #3**
* [2022.01] Confrim that the processor clock frequency cannot be further increased
* [2022.01] Implement the `dev_tty`, `dev_disk`, `cpu_intr` and `cpu_mmu` interfaces in earth
* [2022.01] Load the ELF format grass kernel binary file from the SD card to memory
* [2022.01] Implement the control transfer from earth to grass kernel and then to a user application
* [2022.01] Implement `mkrom` so that creating the bootROM image no longer require Vivado or Docker

**Iteration #2**
* Yeah, I didn't work on this project in most of 2021...
* [2021.12] Create a docker image for portable toolchain setup: [Docker Hub repo](https://hub.docker.com/repository/docker/yhzhang0128/arty-toolchain)
* [2021.12] Reconnect the processor SPI bus controller to the Arty Pmod1 pins
* [2021.12] Implement the SD card initialize, read and write functions
* [2021.12] Increase the processor clock frequency from 32MHz to 65MHz so that read/write blocks become faster

**Iteration #1**
* [2020.12] Increase the processor memory from 24KB to 160KB (128KB + 32KB)
* [2020.12] Confirm that the memory cannot be further increased due to the limitation of Artix-7 35T FPGA chip

**Iteration #0**
* [2020.09] Setup the toolchain provided by SiFive; Compile and run Hello World on Arty
* [2020.09] Test the basic input and print functionalities using the SiFive Metal library
