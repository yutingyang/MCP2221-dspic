#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

void uart1_init(uint32_t baud);
void uart1_putc(char c);
void uart1_puts(const char *s);
bool uart1_getc(char *c);   /* 非阻塞：有資料回傳 true */

#endif
