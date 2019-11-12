#ifndef UART_H
#define UART_H

#include <stdint.h>

void 	  uart_put		 (char *ptr_str);
void 	  uart_putnumU (int i);
void 	  init_uart	   (void);
uint8_t uart_getchar (void);
void		uart_putchar (char ch);

#endif
