/*
 * 硬體設定 —— 換晶片或換腳位時只需要改這個檔案
 *
 * 預設目標：dsPIC33CK256MP508 (例如 dsPIC33CK Curiosity 開發板)
 * 時脈：內部 FRC 8 MHz，不開 PLL  => FOSC = 8 MHz, FCY = 4 MHz
 */
#ifndef CONFIG_H
#define CONFIG_H

#define FOSC 8000000UL
#define FCY  (FOSC / 2)

#define UART_BAUD 115200UL

/* UART 腳位 (dsPIC33CK: RB0 = RP32, RB1 = RP33 ... RB15 = RP47)
 *   dsPIC U1TX  -> RB14 (RP46) -> 接 MCP2221A 的 RX
 *   dsPIC U1RX  <- RB15 (RP47) <- 接 MCP2221A 的 TX
 */
#define UART_RX_RP    47
#define UART_TX_RPOR  RPOR7bits.RP46R
#define UART_TX_TRIS  TRISBbits.TRISB14
#define UART_RX_TRIS  TRISBbits.TRISB15

/* LED (依開發板修改) */
#define LED_TRIS  TRISEbits.TRISE0
#define LED_LAT   LATEbits.LATE0

#endif
