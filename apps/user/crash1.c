/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a program causing a memory exception;
 * Students are asked to modify the grass kernel so that this 
 * program crashes gracefully without crashing the grass kernel
 */

int main() {
    *(int*)(0x1000) = 1;
    return 0;
}
