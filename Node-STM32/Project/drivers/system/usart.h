#ifndef __USART_H
#define	__USART_H

#include "stm32f10x.h"

void USARTx_Config(USART_TypeDef* USARTx, u32 USART_BaudRate);
void USART_RxInt_Config(bool bEnabled, USART_TypeDef* USARTx, uint8_t IRQn);
int USART_ReadInt(USART_TypeDef* USARTx);
int USART_putchar(USART_TypeDef* USARTx, int ch);
int USART_getchar(USART_TypeDef* USARTx);
unsigned char USART_available(USART_TypeDef* USARTx);
void USART_write(USART_TypeDef* USARTx, const uint8_t* data, int len);
void USART_puts(USART_TypeDef* USARTx, char *str);
void USART_printf(USART_TypeDef* USARTx, char *Data, ...);

#endif
