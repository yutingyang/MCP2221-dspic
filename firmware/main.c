/*
 * dsPIC33CK  <->  Adafruit MCP2221A  <->  PC 範例
 *
 * MCP2221A 在電腦上會出現為一個 USB 虛擬序列埠 (CDC)。
 * dsPIC 透過 UART1 與 MCP2221A 相連，電腦用任何序列埠軟體
 * (或 host/mcp2221_host.py) 就能與 dsPIC 對話。
 *
 * 文字指令 (以 \r 或 \n 結尾)：
 *   PING          -> PONG
 *   LED ON|OFF    -> 控制 LED
 *   ECHO <文字>   -> 回傳 <文字>
 *   CNT           -> 回傳開機以來收到的指令數
 *   HELP          -> 列出指令
 */
#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "uart.h"

/* ---- Configuration bits ---- */
#pragma config FNOSC  = FRC       /* 內部 FRC 8 MHz */
#pragma config IESO   = OFF
#pragma config POSCMD = NONE
#pragma config FWDTEN = ON_SW     /* WDT 由軟體控制 (預設關閉) */
#pragma config ICS    = PGD3      /* 依你的燒錄腳位修改 (Curiosity 板為 PGD3) */
#pragma config JTAGEN = OFF

#define LINE_MAX 64

static unsigned int cmd_count = 0;

static void handle_command(char *line)
{
    char out[96];

    cmd_count++;

    if (strcmp(line, "PING") == 0) {
        uart1_puts("PONG\r\n");
    } else if (strcmp(line, "LED ON") == 0) {
        LED_LAT = 1;
        uart1_puts("OK\r\n");
    } else if (strcmp(line, "LED OFF") == 0) {
        LED_LAT = 0;
        uart1_puts("OK\r\n");
    } else if (strncmp(line, "ECHO ", 5) == 0) {
        uart1_puts(line + 5);
        uart1_puts("\r\n");
    } else if (strcmp(line, "CNT") == 0) {
        snprintf(out, sizeof(out), "CNT %u\r\n", cmd_count);
        uart1_puts(out);
    } else if (strcmp(line, "HELP") == 0) {
        uart1_puts("PING | LED ON | LED OFF | ECHO <text> | CNT | HELP\r\n");
    } else {
        cmd_count--;
        snprintf(out, sizeof(out), "ERR unknown command: %s\r\n", line);
        uart1_puts(out);
    }
}

int main(void)
{
    char line[LINE_MAX];
    uint8_t len = 0;
    char c;

    ANSELB = 0;       /* UART 腳位所在的 Port B 設為數位 I/O */

    LED_TRIS = 0;
    LED_LAT  = 0;

    uart1_init(UART_BAUD);

    uart1_puts("\r\ndsPIC33CK + MCP2221A ready. Type HELP\r\n");

    while (1) {
        if (!uart1_getc(&c)) {
            continue;
        }

        if (c == '\r' || c == '\n') {
            if (len > 0) {
                line[len] = '\0';
                handle_command(line);
                len = 0;
            }
        } else if (len < LINE_MAX - 1) {
            line[len++] = c;
        } else {
            len = 0;                    /* 太長就丟棄整行 */
            uart1_puts("ERR line too long\r\n");
        }
    }
    return 0;
}
