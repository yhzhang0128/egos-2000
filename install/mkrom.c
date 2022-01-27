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
char fe310_file[] = "arty_board/fe310_arty.bit";
char output_file[]= "egos_bootROM.mcs";

char mem_fe310[4 * 1024 * 1024];
char mem_earth[4 * 1024 * 1024];
char mem_disk [6 * 1024 * 1024];

void load_fe310();
void load_earth();
void load_disk();
void write_mcs();
void write_mcs_section();
int fe310_size, earth_size, disk_size;

int main() {
    load_fe310();
    load_earth();
    load_disk();
    write_mcs();
    
    return 0;
}

void write_mcs() {
    freopen(output_file, "w", stdout);

    write_mcs_section(mem_fe310, 0x00, fe310_size);
    fprintf(stderr, "[INFO] FE310 wrote\n");
    write_mcs_section(mem_earth, 0x40, earth_size);
    fprintf(stderr, "[INFO] Earth wrote\n");
    write_mcs_section(mem_disk,  0x80, disk_size);
    fprintf(stderr, "[INFO] Disk image wrote\n");
    printf(":00000001FF\n");
    
    fclose(stdout);

    fprintf(stderr, "[INFO] Finish making the bootROM image\n");
}

void write_mcs_section(char* mem, int base, int size) {
    /* using a dummy checksum */
    char chk = 0xff;

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
    printf("[INFO] Disk image file has 0x%x bytes\n", disk_size);
    assert(disk_size <= 6 * 1024 * 1024);

    freopen(disk_file, "r", stdin);
    read(0, mem_disk, disk_size);
    fclose(stdin);
}

void load_earth() {
    struct stat st;
    stat(earth_file, &st);
    earth_size = (int)st.st_size;
    printf("[INFO] Earth binary file has 0x%x bytes\n", earth_size);

    freopen(earth_file, "r", stdin);
    read(0, mem_earth, earth_size);
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
    printf("[INFO] FE310 binary section has 0x%x bytes\n", fe310_size);

    read(0, mem_fe310, fe310_size);
    fclose(stdin);
}
