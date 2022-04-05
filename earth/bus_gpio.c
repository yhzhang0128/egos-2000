/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* 
 * definitions for controlling GPIO0 in FE310
 * see chapter17 of the SiFive FE310-G002 Manual
 */

#define GPIO0_BASE    0x10012000UL
#define GPIO0_IOF_EN  56UL

#define GPIO_REG(offset)   (GPIO0_BASE + offset)
#define GPIO_REGW(offset)  (ACCESS((unsigned int*)GPIO_REG(offset)))

#define GPIO_ENABLE(pin)   GPIO_REGW(GPIO0_IOF_EN) |= (1 << pin);
#define GPIO_DISABLE(pin)  GPIO_REGW(GPIO0_IOF_EN) &= ~(1 << pin);
