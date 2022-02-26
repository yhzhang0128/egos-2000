/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: abstractions of the CPU interrupt/exception interface
 */

#include "egos.h"
#include "earth.h"

static int tmr_id;
static struct metal_cpu *cpu;
static struct metal_interrupt *cpu_int, *tmr_int;

int intr_enable() {
    if (metal_interrupt_enable(tmr_int, tmr_id)) {
        ERROR("Failed to enable timer interrupt");
        return -1;
    }   

    if(metal_interrupt_enable(cpu_int, 0)) {
        ERROR("Failed to enable CPU interrupt");
        return -1;
    }
    return 0;
}

int intr_disable() {
    if (metal_interrupt_disable(tmr_int, tmr_id)) {
        ERROR("Failed to disable timer interrupt");
        return -1;
    }

    if(metal_interrupt_disable(cpu_int, 0)) {
        ERROR("Failed to disable the CPU interrupt");
        return -1;
    }
    return 0;
}

static handler_t handler;
static void handler_wrapper(int id, void* arg) {
    handler(id, arg);
    metal_cpu_set_mtimecmp(cpu, metal_cpu_get_mtime(cpu) + QUANTUM_NCYCLES);
}

int intr_register(int id, handler_t _handler) {
    handler = _handler;
    tmr_id = metal_cpu_timer_get_interrupt_id(cpu);

    INFO("Timer interrupt id is %d", tmr_id);
    if (id != tmr_id) {
        ERROR("Interrupt id %d not supported for register", id);
        return -1;
    }

    if (0 > metal_interrupt_register_handler(tmr_int, tmr_id, handler_wrapper, cpu)) {
        ERROR("Failed at registering timer interrupt handler");
        return -1;
    }
    metal_cpu_set_mtimecmp(cpu, QUANTUM_NCYCLES);

    return 0;
}

int intr_init() {
    cpu = metal_cpu_get(0);
    if(cpu == NULL) {
        ERROR("Unable to get CPU handle");
        return -1;
    }
    
    cpu_int = metal_cpu_interrupt_controller(cpu);
    if(cpu_int == NULL) {
        ERROR("Unable to get CPU interrupt handle");
        return -1;
    }
    metal_interrupt_init(cpu_int);

    int mode = metal_interrupt_get_vector_mode(cpu_int);
    switch (mode) {
    case METAL_DIRECT_MODE:
        INFO("CPU interrupt is in direct mode");
        break;
    case METAL_VECTOR_MODE:
        INFO("CPU interrupt is in vector mode");
        break;
    default:
        ERROR("CPU interrupt is in unknown mode %d", mode);
    }

    tmr_int = metal_cpu_timer_interrupt_controller(cpu);
    if (tmr_int == NULL) {
        ERROR("Unable to get CPU timer interrupt handle");
        return -1;
    }
    metal_interrupt_init(tmr_int);
    
    return 0;
}

