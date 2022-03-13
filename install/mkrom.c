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
 * the output file is in Intel MCS-86 object format
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>

char disk_file[]  = "disk.img";
char earth_file[] = "earth.bin";
char fe310_file[] = "bin/fe310_cpu.bit";

char output_bit[] = "bootROM.bit";
char output_mcs[] = "bootROM.mcs";

char mem_fe310[4 * 1024 * 1024];
char mem_earth[4 * 1024 * 1024];
char mem_disk [6 * 1024 * 1024];

void load_fe310();
void load_earth();
void load_disk();
void write_bitstream();
void write_intel_mcs();
void write_section(char* mem, int base, int size);

int fe310_size, earth_size, disk_size;

int main() {
    load_fe310();
    load_earth();
    load_disk();
    write_bitstream();
    write_intel_mcs();
    
    return 0;
}

void write_bitstream() {
    freopen(output_bit, "a", stdout);
    int length = (4 + 4 + 6) * 1024 * 1024;
    putw(length, stdout);

    for (int i = 0; i < 4 * 1024 * 1024; i++)
        putchar(mem_fe310[i]);
    fprintf(stderr, "[INFO] FE310 written\n");

    for (int i = 0; i < 4 * 1024 * 1024; i++)
        putchar(mem_earth[i]);
    fprintf(stderr, "[INFO] Earth written\n");

    for (int i = 0; i < 6 * 1024 * 1024; i++)
        putchar(mem_disk[i]);
    fprintf(stderr, "[INFO] Disk image written\n");

    fclose(stdout);
    fprintf(stderr, "[INFO] Finish making the bootROM bitstream\n");
}

void write_intel_mcs() {
    freopen(output_mcs, "w", stdout);

    write_section(mem_fe310, 0x00, fe310_size);
    fprintf(stderr, "[INFO] FE310 written\n");

    write_section(mem_earth, 0x40, earth_size);
    fprintf(stderr, "[INFO] Earth written\n");

    int paging_size = 1024 * 1024;
    write_section(mem_disk + paging_size,
                  0x80 + 0x10,
                  disk_size - paging_size);
    fprintf(stderr, "[INFO] Disk image written\n");
    printf(":00000001FF\n");
    
    fclose(stdout);
    fprintf(stderr, "[INFO] Finish making the bootROM mcs image\n");
}

void write_section(char* mem, int base, int size) {
    /* using a dummy checksum */
    char chk;

    int ngroups = (size >> 16) + 1;
    for (int i = 0; i < ngroups; i++) {
        printf(":02000004%.4X%.2X\n", i + base, chk & 0xff);
        for (int j = 0; j < 0x10000; j += 16) {
            printf(":10%.4X00", j);
            for (int k = 0; k < 16; k++)
                printf("%.2X", mem[i * 0x10000 + j + k] & 0xff);
            printf("%.2X\n", chk & 0xff);
            if (i * 0x10000 + j + 16 >= size)
                return;
        }
    }    
}

void load_disk() {
    struct stat st;
    stat(disk_file, &st);
    disk_size = (int)st.st_size;
    printf("[INFO] Disk  image  has 0x%.6x bytes\n", disk_size);
    assert(disk_size <= 6 * 1024 * 1024);

    freopen(disk_file, "r", stdin);
    int nread = 0;
    while (nread < disk_size)
        nread += read(0, mem_disk + nread, disk_size - nread);
    fclose(stdin);
}

void load_earth() {
    struct stat st;
    stat(earth_file, &st);
    earth_size = (int)st.st_size;
    printf("[INFO] Earth binary has 0x%.6x bytes\n", earth_size);

    freopen(earth_file, "r", stdin);
    int nread = 0;
    while (nread < earth_size)
        nread += read(0, mem_earth + nread, earth_size - nread);
    fclose(stdin);
}

void load_fe310() {
    /* load the fe310 binary file */
    struct stat st;
    stat(fe310_file, &st);
    int len = (int)st.st_size;
    //printf("[INFO] FE310 binary file has %d bytes\n", len);

    /* load header */
    freopen(fe310_file, "r", stdin);
    int first, second;
    first = getchar();
    second = getchar();
    int length = (first << 8) + second;
    for (int i = 0, tmp; i < length; i++)
        tmp = getchar();

    /* load key length */
    first = getchar();
    second = getchar();
    int key_len = (first << 8) + second;
    assert(key_len == 1);

    while (1) {
        int key = getchar();
        first = getchar();
        second = getchar();
        if ((char)key == 'e')
            break;
        
        length = (first << 8) + second;
        for (int i = 0, tmp; i < length; i++)
            tmp = getchar();
    }

    int third, fourth;
    third = getchar();
    fourth = getchar();
    fe310_size = (first << 24) + (second << 16) + (third << 8) + fourth;
    printf("[INFO] FE310 binary has 0x%.6x bytes\n", fe310_size);

    for (int i = 0; i < fe310_size; i++)
        mem_fe310[i] = getchar();
    fclose(stdin);
}
