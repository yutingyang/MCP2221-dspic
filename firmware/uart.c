/*
 * UART1 驅動 (dsPIC33CK 系列)
 * 接收使用中斷 + 環形緩衝區，傳送使用輪詢。
 */
#include <xc.h>
#include "config.h"
#include "uart.h"

#define RX_BUF_SIZE 64u   /* 必須是 2 的次方 */

static volatile char rx_buf[RX_BUF_SIZE];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;

void uart1_init(uint32_t baud)
{
    /* --- PPS 腳位對應 --- */
    __builtin_write_RPCON(0x0000);      /* 解鎖 PPS */
    RPINR18bits.U1RXR = UART_RX_RP;     /* U1RX <- RPn */
    UART_TX_RPOR = 1;                   /* RPn <- U1TX (功能碼 1) */
    __builtin_write_RPCON(0x0800);      /* 重新上鎖 */

    UART_TX_TRIS = 0;
    UART_RX_TRIS = 1;

    U1MODE  = 0;
    U1MODEH = 0;
    U1STA   = 0;
    U1STAH  = 0;

    /* 分數式鮑率產生器：Baud = FP / BRG，FP = FOSC/2 = FCY */
    U1MODEHbits.BCLKMOD = 1;
    U1BRG = (uint16_t)((FCY + baud / 2) / baud);

    U1MODEbits.MOD = 0;                 /* 8-N-1 非同步模式 */

    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 1;                /* 每收到 1 byte 觸發中斷 */

    U1MODEbits.UARTEN = 1;
    U1MODEbits.UTXEN  = 1;
    U1MODEbits.URXEN  = 1;
}

void uart1_putc(char c)
{
    while (U1STAHbits.UTXBF) { }        /* 等待 TX FIFO 有空位 */
    U1TXREG = c;
}

void uart1_puts(const char *s)
{
    while (*s) {
        uart1_putc(*s++);
    }
}

bool uart1_getc(char *c)
{
    if (rx_head == rx_tail) {
        return false;
    }
    *c = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1u) & (RX_BUF_SIZE - 1u);
    return true;
}

void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void)
{
    while (!U1STAHbits.URXBE) {         /* 把 RX FIFO 讀乾淨 */
        char c = U1RXREG;
        uint8_t next = (rx_head + 1u) & (RX_BUF_SIZE - 1u);
        if (next != rx_tail) {          /* 緩衝區滿則丟棄 */
            rx_buf[rx_head] = c;
            rx_head = next;
        }
    }
    if (U1STAbits.OERR) {
        U1STAbits.OERR = 0;
    }
    IFS0bits.U1RXIF = 0;
}
