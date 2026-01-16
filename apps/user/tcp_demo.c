/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: a simple TCP demo over WiFi (Arty board only)
 * This app connects to a WiFi with a password, and then sends a
 * hello-world string to a destination IP+TCP port. This app can
 * only run on the Arty boards with Pmod ESP32 inserted as shown
 * in the README picture. Run tcp_demo, and you shall see:
    ➜ /home/yunhao tcp_demo
    [CRITICAL] Press the button on Pmod ESP32
    ESP32 runs AP command:
    AT+CWMODE=1

    OK
    ESP32 runs AP command:
    AT+CWJAP="3602","yunhao0128"
    WIFI CONNECTED
    WIFI GOT IP

    OK
    ESP32 runs AP command:
    AT+CIPSTART="TCP","192.168.0.212",8002
    CONNECT

    OK
    ESP32 runs AP command:
    AT+CIPSEND=22

    OK
    ESP32 runs AP command:
    AT+CIPCLOSE
    CLOSED

    OK
 * This app connects to WiFi "3602" with password "yunhao0128",
 * establishs a TCP connection to 192.168.0.212:8002, and sends
 * string "Hello from egos-2000!\n" through the TCP connection.
 * Here is a demo video: https://youtube.com/shorts/h7HIpzuhEJo
 */

#include "app.h"

void esp32_wait(const char* ack, int print) {
    char reply[1024] = {0};
    for (int i = 0; i < 1024; i++) {
        /* Stop if we get the ack string from ESP32. */
        if (i >= strlen(ack) && !strcmp(reply + i - strlen(ack), ack)) break;
        /* Get one more char from ESP32. */
        while (REGW(WIFI_BASE, 8UL));
        reply[i]              = REGW(WIFI_BASE, 0) & 0xFF;
        REGW(WIFI_BASE, 16UL) = 2;
    }
    if (print) printf("\x1B[1;32mESP32 runs AP command:\x1B[1;0m\n\r%s", reply);
}

int main() {
    CRITICAL("Press the button on Pmod ESP32");
    esp32_wait("ready\r\n", 0);

    /* This app runs 5 AP commands on ESP32:
     *     AT+CWMODE   sets ESP32 to the WiFi station mode;
     *     AT+CWJAP    connects to a WiFi with a password;
     *     AT+CIPSTART establishes a TCP connection;
     *     AT+CIPSEND  sends a string to the TCP connection;
     *     AT+CIPCLOSE closes the TCP connection.
     *
     * Read this document for more details on AP commands:
     * http://espressif.com/sites/default/files/documentation/esp32_at_instruction_set_and_examples_en.pdf
     */
    const char* AP_cmds[] = {"AT+CWMODE=1\r\n",
                             "AT+CWJAP=\"3602\",\"yunhao0128\"\r\n",
                             "AT+CIPSTART=\"TCP\",\"192.168.0.212\",8002\r\n",
                             "AT+CIPSEND=22\r\n",
                             "Hello from egos-2000!\n",
                             "AT+CIPCLOSE\r\n"};

    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < strlen(AP_cmds[i]); j++) {
            while (REGW(WIFI_BASE, 4UL));
            REGW(WIFI_BASE, 0)    = AP_cmds[i][j];
            REGW(WIFI_BASE, 16UL) = 1;
        }

        esp32_wait("OK\r\n", i != 4); /* AP_cmds[4] is not an AP command. */
    }

    return 0;
}
