#ifndef __COMMON__H__
#define __COMMON__H__

#include "stm32f10x.h"
#include "pin.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define ARRAY_SIZE(_Array) (sizeof(_Array) / sizeof(_Array[0]))

#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(!!(COND))*2-1]
// token pasting madness:
#define COMPILE_TIME_ASSERT3(X,L) STATIC_ASSERT(X,static_assertion_at_line_##L)
#define COMPILE_TIME_ASSERT2(X,L) COMPILE_TIME_ASSERT3(X,L)
#define COMPILE_TIME_ASSERT(X)    COMPILE_TIME_ASSERT2(X,__LINE__)

#define DBG_MSG(format, ...) printf("[Debug]%s: " format "\r\n", __func__, ##__VA_ARGS__)
#define ERR_MSG(format, ...) printf("[Error]%s: " format "\r\n", __func__, ##__VA_ARGS__)
 
void RCC_GPIOClockCmd(GPIO_TypeDef* GPIOx, FunctionalState state);
void RCC_USARTClockCmd(USART_TypeDef* USARTx, FunctionalState state);
void RCC_TIMClockCmd(TIM_TypeDef* TIMx, FunctionalState NewState);
void Chip_GetUniqueID(uint32_t ChipUniqueID[3]);
void Timer_16bit_Calc(int freq, uint16_t *period, uint16_t *prescaler);

#endif /* __COMMON__H__ */