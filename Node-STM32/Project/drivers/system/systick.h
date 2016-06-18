#ifndef SYSTICK_H__
#define SYSTICK_H__

typedef unsigned long long SysTick_t;

void SysTick_Init(void);
void IncSysTickCounter(void);
SysTick_t GetSystemTick(void);
void Delay_ms(unsigned int ms);
void Delay_us(unsigned int us);

#endif
