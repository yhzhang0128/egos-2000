This document describes the teaching plan, architecture and development history of egos-2000.

## Teaching plan (EGOS + EGOS2000)

We use EGOS to teach our Practicum in Operating Systems (CS4411) at Cornell, a 2-credit course focusing solely on projects.
There are five mandatory projects:

|    | Description                     | Concepts to teach                                           | Platform  |
|----|---------------------------------|-------------------------------------------------------------|-----------|
| P0 | Queue                           | memory, pointer, instruction and stack pointers             | Linux/Mac |
| P1 | User-level threading            | thread, context switch, synchronization                     | Linux/Mac |
| P2 | Multi-level feedback queue      | timer and quantum, scheduling, priority                     | Linux/Mac |
| P3 | Memory management               | control register, exception handling, privilege level       | Arty FPGA |
| P5 | File system                     | inode layer, directory layer, layered file system           | Arty FPGA |

And some optional projects:
|    | Description                     | Concepts to teach                                                  | Platform  |
|----|---------------------------------|--------------------------------------------------------------------|-----------|
| P4 | SD card driver                  | I/O bus, memory-mapped bus controller, device driver               | Arty FPGA |
| P6 | Networking                      | IP layer (Ethernet driver with SPI), transportation layer (UDP)    | Arty FPGA |
| P7 | Graphic user interface          | VGA driver with GPIO ([example](https://digilent.com/reference/learn/programmable-logic/tutorials/arty-pmod-vga-demo/start))                        | Arty FPGA |

## Architecture

**Earth layer (hardware specific)**
* `earth/dev_disk`: SD card (touched by P4)
* `earth/dev_tty`: keyboard input and tty output
* `earth/cpu_intr`: interrupt and exception handling (touched by P3)
* `earth/cpu_mmu`: memory paging and address translation (touched by P3)

**Grass layer (hardware independent)**
* `grass/timer`: control timer registers
* `grass/process`: manage process data structures (touched by P1)
* `grass/scheduler`: preemptive scheduling and inter-process communication (touched by P2)

**Application layer**
* `app/system/proc_file`: manage an inode layer on the SD card (touched by P5)
* `app/system/proc_dir`: manage the directory layer on top of inode layer (touched by P5)
* `app/system/proc_shell`: interactive user interface
* user commands: `pwd`, `clear`, `killall`, `ls`, `cat`, `echo`, `clock`

Every layer has a dedicated memory region as described below (and in `library/egos.h`). The complete memory layout is described in Chapter 4 of the FE310 manual in this repository.

**Selected RAM regions**

| Base        | Top         | Attributes | Description       | Notes                                                      |
|-------------|-------------|------------|-------------------|------------------------------------------------------------|
| 0x0800_0000 | 0x0800_2FFF | RWX A      | ITIM, 12KB        | Earth layer bss, data and heap                             |
| 0x0800_3000 | 0x0800_4FFF | RWX A      | ITIM, 8KB         | Grass layer code, bss, data and heap                       |
| 0x0800_5000 | 0x0800_7FFF | RWX A      | ITIM, 12KB        | App code, bss, data and heap                               |
| ......      | ......      | ......     | ......            |                                                            |
| 0x2000_0000 | 0x203F_FFFF | R XC       | Flash ROM, 4MB    | FPGA binary of the FE310 RISC-V processor                  |
| 0x2040_0000 | 0x207F_FFFF | R XC       | Flash ROM, 4MB    | Earth layer code and rodata                                |
| 0x2080_0000 | 0x20BF_FFFF | R XC       | Flash ROM, 4MB    | Disk image (disk.img produced by mkrom)                    |
| ......      | ......      | ......     | ......            |                                                            |
| 0x8000_0000 | 0x8000_1FFF | RW- A      | DTIM, 8KB         | App stack                                                  |
| 0x8000_2000 | 0x8000_3FFF | RW- A      | DTIM, 8KB         | Earth and grass stack                                      |
| 0x8000_4000 | 0x8001_FFFF | RW- A      | DTIM, 112KB       | MMU cache of physical frames for suspended grass processes |

The first 1MB of the microSD card is used as 256 physical frames by the MMU for paging.

**Selected memory-mapped I/O regions**

| Base        | Top         | Attributes | Description   | Notes                                 |
|-------------|-------------|------------|---------------|---------------------------------------|
| 0x1001_4000 | 0x1001_4FFF | RW A       | QSPI 0        | Control the 16MB on-board flash ROM   |
| 0x1002_4000 | 0x1002_4FFF | RW A       | SPI 1         | Control the Pmod1 (microSD card) pins |

## Software development history

**Iteration #６**

* [2022.03] Add support of background shell commands and `killall`
* [2022.03] Add servers in `library/syscall`; Cleanup access to the file system
* [2022.03] Cleanup the code controlling UART and SPI; Remove dependency on the Freedom Metal library
* [2022.04] Add a simple boot loader `earth/earth.S`; Remove dependency on the Freedom Metal library
* [2022.04] Add `_write()` and `_sbrk()` in `library/libc`; Remove the Freedom Metal library entirely

**Iteration #5**
* [2022.02.26] Implement timer reset and preemptive scheduling
* [2022.02.27] Implement inter-process communication
* [2022.02.28] Implement kernel processes: GPID_PROCESS, GPID_FILE, GPID_DIR, GPID_SHELL
* [2022.03.01] Implement system calls and 4 shell commands: `pwd`, `ls`, `cat` and `echo`


**Iteration #4**
* [2022.02.10] Read Chapter 1, 2 and 3 of RISC-V manual
* [2022.02.17] Read Chapter 8, 9 and 10 of FE310 manual

**Iteration #3**
* [2022.01] Confrim that the processor clock frequency cannot be further increased
* [2022.01] Implement the `dev_tty`, `dev_disk`, `cpu_intr` and `cpu_mmu` interfaces in earth
* [2022.01] Load the ELF format grass kernel binary file from the SD card to memory
* [2022.01] Implement the control transfer from earth to grass kernel and then to a user application
* [2022.01] Implement `mkrom` so that creating the bootROM image no longer require Vivado or Docker

**Iteration #2**
* [2021.12] Create a docker image for portable toolchain setup: [Docker Hub repo](https://hub.docker.com/repository/docker/yhzhang0128/arty-toolchain)
* [2021.12] Reconnect the processor SPI bus controller to the Arty Pmod1 ports
* [2021.12] Implement the SD card initialize, read_block and write_block functions
* [2021.12] Increase the processor clock frequency from 32MHz to 65MHz so that read/write blocks becomes faster
* Yeah, I didn't do much for this project in most of 2021...

**Iteration #1**
* [2020.12] Increase the processor memory from 32KB to 160KB (128KB + 32KB)
* [2020.12] Confirm that the memory cannot be further increased due to the Artix-7 35T limits

**Iteration #0**
* [2020.09] Setup the toolchain; Compile and run Hello World on Arty
* [2020.09] Test the basic input and print functionalities using the Freedom Metal library
