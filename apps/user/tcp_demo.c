/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple TCP demo over WiFi
 */

#include "app.h"

#define ESP32_BASE    0xF0002000UL /* TODO: this conflicts with NIC_BASE */
#define ESP32_TXFULL  REGW(ESP32_BASE, 4UL)
#define ESP32_RXEMPTY REGW(ESP32_BASE, 8UL)
#define ESP32_EVPEND  REGW(ESP32_BASE, 16UL)

void esp32_getc(char* c) {
    while (ESP32_RXEMPTY);
    *c           = REGW(ESP32_BASE, 0) & 0xFF;
    ESP32_EVPEND = 2;
}

void esp32_putc(char c) {
    while (ESP32_TXFULL);
    REGW(ESP32_BASE, 0) = c;
    ESP32_EVPEND        = 1;
}

void esp32_wait(const char* ack) {
    char rep[1024] = {0};
    for (int i = 0; i < 1024; i++) {
        if (i >= strlen(ack) && strcmp(rep + i - strlen(ack), ack) == 0) break;
        esp32_getc(rep + i);
    }
    printf("# Get ESP32 reply:\n\r%s", rep);
}

int main() {
    CRITICAL("Press the button on Pmod ESP32");
    esp32_wait("ready\r\n");

    /* AT+CWMODE   sets ESP32 to WiFi station mode (mode=1)
     * AT+CWJAP    connects to a WiFi with a password
     * AT+CIPSTART establishes a TCP connection
     * AT+CIPSEND  tells ESP32 the length of string hello
     *
     * Read this document for more details of AP commands:
     * http://espressif.com/sites/default/files/documentation/esp32_at_instruction_set_and_examples_en.pdf
     */
    char* AP_cmds[] = {
        "AT+CWMODE=1\r\n", "AT+CWJAP=\"3602\",\"yunhao0128\"\r\n",
        "AT+CIPSTART=\"TCP\",\"192.168.0.212\",8002\r\n", "AT+CIPSEND=23\r\n"};

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < strlen(AP_cmds[i]); j++) esp32_putc(AP_cmds[i][j]);
        esp32_wait("OK\r\n");
    }

    /* Send the hello string through the TCP connection over WiFi. */
    char* hello = "Hello from egos-2000!\n\r";
    for (int i = 0; i < strlen(hello); i++) esp32_putc(hello[i]);

    return 0;
}
