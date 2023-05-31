/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: create the bootROM image files
 * The ROM on the Arty board has 16MB:
 *     the first 4MB holds the FE310 processor;
 *     the next  4MB holds the earth layer binary executable;
 *     the next  4MB holds the disk image produced by mkfs;
 *     the last  4MB is currently unused.
 * The output is in binary (bootROM.bin) and Intel MCS-86 (bootROM.mcs) format.
 * The MCS-86 format file is only for debugging and not really necessary.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>

#define SIZE_4MB  4 * 1024 * 1024

int fe310_size, earth_size, disk_size;
char mem_fe310[SIZE_4MB], mem_earth[SIZE_4MB], mem_disk [SIZE_4MB];

int load_file(char* file_name, char* print_name, char* dst) {
    struct stat st;
    stat(file_name, &st);
    printf("[INFO] %s has 0x%.6x bytes\n", print_name, (int)st.st_size);

    freopen(file_name, "r", stdin);
    for (int nread = 0; nread < st.st_size; )
        nread += read(0, dst + nread, st.st_size - nread);
    fclose(stdin);

    return st.st_size;
}

int main(int argc, char** argv) {
    fe310_size = load_file("fpga/freedom/fe310_cpu_a7_35t.bin", "FE310 binary", mem_fe310);
    // Use the one below if you are using an Arty A7-100T board
    //fe310_size = load_file("fpga/freedom/fe310_cpu_a7_100t.bin", "FE310 binary", mem_fe310);
    earth_size = load_file("earth.bin",                  "Earth binary", mem_earth);
    disk_size =  load_file("disk.img",                   "Disk  image ", mem_disk);

    assert(fe310_size <= SIZE_4MB && earth_size <= SIZE_4MB && disk_size  == SIZE_4MB);

    freopen("bootROM.bin", "w", stdout);
    for (int i = 0; i < SIZE_4MB; i++) putchar(mem_fe310[i]);
    for (int i = 0; i < SIZE_4MB; i++) putchar(mem_earth[i]);
    for (int i = 0; i < SIZE_4MB; i++) putchar(mem_disk[i]);
    fclose(stdout);

    fprintf(stderr, "[INFO] Finish making the bootROM binary (tools/bootROM.bin)\n");
    /* write_intel_mcs(); */
    return 0;
}


/* #define SIZE_1MB  1 * 1024 * 1024 */
/* void write_mcs_section(char* mem, int base, int size); */
/* void write_intel_mcs() { */
/*     freopen("bootROM.mcs", "w", stdout); */

/*     write_mcs_section(mem_fe310, 0x00, fe310_size); */
/*     write_mcs_section(mem_earth, 0x40, earth_size); */

/*     int paging = SIZE_1MB; */
/*     write_mcs_section(mem_disk + paging, */
/*                       0x80 + (paging >> 16), disk_size - paging); */
/*     printf(":00000001FF\n"); */
    
/*     fclose(stdout); */
/*     fprintf(stderr, "[INFO] Finish making the bootROM mcs image\n"); */
/* } */

/* void write_mcs_section(char* mem, int base, int size) { */
/*     for (int i = 0; i < (size >> 16) + 1; i++) { */
/*         printf(":02000004%.4X%.2X\n", i + base, 0xFF); */
/*         for (int j = 0; j < 0x10000; j += 16) { */
/*             printf(":10%.4X00", j); */
/*             for (int k = 0; k < 16; k++) */
/*                 printf("%.2X", mem[i * 0x10000 + j + k] & 0xFF); */
/*             /\* Use a dummy checksum *\/ */
/*             printf("%.2X\n", 0xFF); */
/*             if (i * 0x10000 + j + 16 >= size) return; */
/*         } */
/*     }     */
/* } */
