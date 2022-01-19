/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: abstractions of the CPU interrupt/exception interface
 */

#include "earth.h"

static struct metal_interrupt *cpu_int;

int intr_init() {
    struct metal_cpu *cpu = metal_cpu_get(0);
    if(!cpu) {
        ERROR("Unable to get CPU handle");
        return -1;
    }
    
    cpu_int = metal_cpu_interrupt_controller(cpu);
    if(!cpu_int) {
        ERROR("Unable to get CPU interrupt handle");
        return -1;
    }
    metal_interrupt_init(cpu_int);

    
    return 0;
}

int intr_enable() {
    if(metal_interrupt_enable(cpu_int, 0) != 0) {
        ERROR("Failed to enable the CPU interrupt");
        return -1;
    }
    return 0;
}

int intr_disable() {
    if(metal_interrupt_disable(cpu_int, 0) != 0) {
        ERROR("Failed to disable the CPU interrupt");
        return -1;
    }
    return 0;
}
    
