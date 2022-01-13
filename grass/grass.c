/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: the grass kernel that initializes the process control block
 * and spawns a few key kernel processes
 */


int global_var1;
int global_var2;
int global_var3 = 1;

int main() {
    global_var1++;
    global_var2++;
    global_var3++;
    return 0;
}
