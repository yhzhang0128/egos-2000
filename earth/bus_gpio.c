/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: definitions for GPIO0 in FE310
 * see chapter17 of the SiFive FE310-G002 Manual
 *
 * The commented functions can control the LED lights on the Arty board.
 */

#define GPIO0_BASE        0x10012000UL
#define GPIO0_IOF_ENABLE  56UL
#define GPIO0_IOF_SELECT  60UL

/*
#include "egos.h"
#define LED0_RED    1
#define LED0_GREEN  2
#define LED0_BLUE   3

static void led_on(int led) {
    REGW(GPIO0_BASE, GPIO0_IOF_ENABLE) |= (1 << led);
    REGW(GPIO0_BASE, GPIO0_IOF_SELECT) |= (1 << led);
}

static void led_off(int led) {
    REGW(GPIO0_BASE, GPIO0_IOF_ENABLE) &= ~(1 << led);
    REGW(GPIO0_BASE, GPIO0_IOF_SELECT) &= ~(1 << led);
}
*/
