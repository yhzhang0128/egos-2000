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
#define GPIO0_IOF_SEL 60UL

#define GPIO_REG(offset)   (GPIO0_BASE + offset)
#define GPIO_REGW(offset)  (ACCESS((unsigned int*)GPIO_REG(offset)))

/* #define LED0_RED    1 */
/* #define LED0_GREEN  2 */
/* #define LED0_BLUE   3 */

/* #define LED_ON(x)  GPIO_REGW(GPIO0_IOF_EN) |= (1 << x); GPIO_REGW(GPIO0_IOF_SEL) |= (1 << x); */
/* #define LED_OFF(x) GPIO_REGW(GPIO0_IOF_EN) &= ~(1 << x); GPIO_REGW(GPIO0_IOF_SEL) &= ~(1 << x); */