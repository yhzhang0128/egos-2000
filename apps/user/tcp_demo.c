/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple TCP demo over WiFi
 */

#include "app.h"

#define ESP32_BASE 0xF0002000UL /* TODO: this conflicts with NIC_BASE */

void esp32_wait(const char* ack, int print) {
    char reply[1024] = {0};
    for (int i = 0; i < 1024; i++) {
        /* Stop if we get the ack string from ESP32. */
        if (i >= strlen(ack) && !strcmp(reply + i - strlen(ack), ack)) break;
        /* Get one more char from ESP32. */
        while (REGW(ESP32_BASE, 8UL));
        reply[i]               = REGW(ESP32_BASE, 0) & 0xFF;
        REGW(ESP32_BASE, 16UL) = 2;
    }
    if (print) printf("\x1B[1;32mESP32 runs AP command:\x1B[1;0m\n\r%s", reply);
}

int main() {
    CRITICAL("Press the button on Pmod ESP32");
    esp32_wait("ready\r\n", 0);

    /* This app runs 4 AP commands on ESP32:
     *     AT+CWMODE   sets ESP32 to the WiFi station mode;
     *     AT+CWJAP    connects to a WiFi with a password;
     *     AT+CIPSTART establishes a TCP connection;
     *     AT+CIPSEND  asks ESP32 to send a 23-byte string.
     *
     * Read this document for more details on AP commands:
     * http://espressif.com/sites/default/files/documentation/esp32_at_instruction_set_and_examples_en.pdf
     */
    char* AP_cmds[] = {"AT+CWMODE=1\r\n",
                       "AT+CWJAP=\"3602\",\"yunhao0128\"\r\n",
                       "AT+CIPSTART=\"TCP\",\"192.168.0.212\",8002\r\n",
                       "AT+CIPSEND=23\r\n", "Hello from egos-2000!\n\r"};

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < strlen(AP_cmds[i]); j++) {
            while (REGW(ESP32_BASE, 4UL));
            REGW(ESP32_BASE, 0)    = AP_cmds[i][j];
            REGW(ESP32_BASE, 16UL) = 1;
        }

        if (i != 4) esp32_wait("OK\r\n", 1);
    }

    return 0;
}
