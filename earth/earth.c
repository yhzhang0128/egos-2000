#include <stdio.h>
#include <metal/uart.h>

#include "log.h"

int uart_init();
struct metal_uart* uart;

int main() {
    if (uart_init())
        return -1;

    while (1) {
        printf("This is the earthbox. Enter a key:\r\n");

        int c;
        for (c = -1; c == -1; metal_uart_getc(uart, &c));

        SUCCESS("Got char: 0x%.2x", (char) c);
    }
    return 0;
}

int uart_init() {
    uart = metal_uart_get_device(0);
    metal_uart_init(uart, 115200);
    for (int i = 0; i < 2000000; i++);
    
    if (!uart) {
        ERROR("Unable to get uart handle");
        return -1;
    }

    // clear up some initial input to uart
    while (1) {
        int tmp;
        int r = metal_uart_getc(uart, &tmp);
        if (tmp == -1)
            break;
    }
    return 0;
}
    
