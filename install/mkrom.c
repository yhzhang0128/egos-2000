/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: create the bootROM image file (egos_bootROM.mcs)
 * the bootROM has 16MB
 * the first 4MB is reserved for the FE310 processor
 * the second 4MB is reserved for the earth layer binary
 * the last 8MB is reserved for the disk image
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>

char disk_file[]  = "disk.img";
char earth_file[] = "earth.bin";
char fe310_file[] = "arty_board/fe310_arty.bit";

char mem_fe310[4 * 1024 * 1024];
char mem_earth[4 * 1024 * 1024];
char mem_disk [8 * 1024 * 1024];

void load_fe310();
void load_earth();
void load_disk();
int fe310_size, earth_size, disk_size;

int main() {
    load_fe310();
    load_earth();
    load_disk();
    return 0;
}

void load_disk() {
    struct stat st;
    stat(disk_file, &st);
    disk_size = (int)st.st_size;
    printf("[INFO] Disk image file has 0x%x bytes\n", disk_size);

    freopen(earth_file, "r", stdin);
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
    printf("[INFO] FE310 binary file has %d bytes\n", len);

    /* load header */
    freopen(fe310_file, "r", stdin);
    int first, second;
    first = getchar();
    second = getchar();
    int length = (first << 8) + second;
    //printf("[INFO] The header has length %d bytes\n", length);
    for (int i = 0, tmp; i < length; i++)
        tmp = getchar();

    /* load key length */
    first = getchar();
    second = getchar();
    int key_len = (first << 8) + second;
    //printf("[INFO] Keys have length %d byte\n", key_len);
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
        //printf("[INFO] Section %c has length %d\n", (char)key, length);
    }

    int third, fourth;
    third = getchar();
    fourth = getchar();
    fe310_size = (first << 24) + (second << 16) + (third << 8) + fourth;
    printf("[INFO] FE310 binary section has 0x%x bytes\n", fe310_size);

    read(0, mem_fe310, fe310_size);
    fclose(stdin);
}
