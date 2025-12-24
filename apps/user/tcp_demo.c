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

char rep[4096];
void esp32_get_reply() {
    memset(rep, 0, 4096);
    for (int i = 0; i == 0 || rep[i - 1] != '\n'; i++) esp32_getc(rep + i);
}

void esp32_wait_ok() {
    do {
        esp32_get_reply();
        if (rep[0] != '\r') printf("Get ESP32 reply=%s", rep);
    } while (strcmp(rep, "OK\r\n") != 0);
}

int main() {
    CRITICAL("Press the button on Pmod ESP32");
    while (strcmp(rep, "ready\r\n") != 0) esp32_get_reply();

    SUCCESS("Set ESP32 to WiFi Station mode");
    const char* mode_cmd = "AT+CWMODE=1\r\n";
    for (int i = 0; i < 13; i++) esp32_putc(mode_cmd[i]);
    esp32_wait_ok();

    SUCCESS("Connect to WiFi");
    const char* wifi_cmd = "AT+CWJAP=\"3602\",\"yunhao0128\"\r\n";
    for (int i = 0; i < 30; i++) esp32_putc(wifi_cmd[i]);
    esp32_wait_ok();

    SUCCESS("Establish a TCP connection to 192.168.0.212:8002");
    const char* tcp_cmd = "AT+CIPSTART=\"TCP\",\"192.168.0.212\",8002\r\n";
    for (int i = 0; i < 40; i++) esp32_putc(tcp_cmd[i]);
    esp32_wait_ok();

    SUCCESS("Send a hello-world string through TCP");
    const char* hello    = "Hello, World!\n";
    const char* send_cmd = "AT+CIPSEND=14\r\n";
    for (int i = 0; i < 15; i++) esp32_putc(send_cmd[i]);
    esp32_wait_ok();
    for (int i = 0; i < 14; i++) esp32_putc(hello[i]);

    return 0;
}
