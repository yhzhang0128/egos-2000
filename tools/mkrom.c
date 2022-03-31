/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: create the bootROM image file (egos_bootROM.mcs)
 * the bootROM has 16MB
 *     the first 4MB is reserved for the FE310 processor
 *     the next 4MB is reserved for the earth layer binary
 *     the next 6MB is reserved for the disk image
 *     the last 2MB is currently unused
 * the output file is in binary and Intel MCS-86 format
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>

char mem_fe310[4 * 1024 * 1024];
char mem_earth[4 * 1024 * 1024];
char mem_disk [6 * 1024 * 1024];
int fe310_size, earth_size, disk_size;

void write_binary();
void write_intel_mcs();
int load_file(char* file_name, char* print_name, char* dst);

int main() {
    fe310_size = load_file("fe310_cpu.bin", "FE310 binary", mem_fe310);
    earth_size = load_file("earth.bin", "Earth binary", mem_earth);
    disk_size = load_file("disk.img", "Disk  image ", mem_disk);

    assert(fe310_size <= 4 * 1024 * 1024);
    assert(earth_size <= 4 * 1024 * 1024);
    assert(disk_size  <= 6 * 1024 * 1024);

    write_binary();
    write_intel_mcs();
    return 0;
}

void write_binary() {
    freopen("bootROM.bin", "w", stdout);

    write(1, mem_fe310, 4 * 1024 * 1024);
    write(1, mem_earth, 4 * 1024 * 1024);
    write(1, mem_disk,  6 * 1024 * 1024);

    fclose(stdout);
    fprintf(stderr, "[INFO] Finish making the bootROM binary\n");
}

void write_mcs_section(char* mem, int base, int size);
void write_intel_mcs() {
    freopen("bootROM.mcs", "w", stdout);

    write_mcs_section(mem_fe310, 0x00, fe310_size);
    write_mcs_section(mem_earth, 0x40, earth_size);

    int paging_size = 1024 * 1024;
    write_mcs_section(mem_disk + paging_size,
                      0x80 + 0x10,
                      disk_size - paging_size);
    printf(":00000001FF\n");
    
    fclose(stdout);
    fprintf(stderr, "[INFO] Finish making the bootROM mcs image\n");
}

void write_mcs_section(char* mem, int base, int size) {
    /* using a dummy checksum */
    int ngroups = (size >> 16) + 1;
    for (int i = 0; i < ngroups; i++) {
        printf(":02000004%.4X%.2X\n", i + base, 0xff);
        for (int j = 0; j < 0x10000; j += 16) {
            printf(":10%.4X00", j);
            for (int k = 0; k < 16; k++)
                printf("%.2X", mem[i * 0x10000 + j + k] & 0xff);
            printf("%.2X\n", 0xff);
            if (i * 0x10000 + j + 16 >= size) return;
        }
    }    
}

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
