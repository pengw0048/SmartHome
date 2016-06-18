#include "stm32f10x.h"
#include "systick.h"

static volatile SysTick_t systemTickCounter = 0;

void SysTick_Init(void)
{

	/* SystemFrequency / 1000    1ms中断一次
	* SystemFrequency / 100000  10us中断一次
	* SystemFrequency / 1000000 1us中断一次
	*/

	//  if (SysTick_Config(SystemFrequency / 1000)) // ST3.0.0库版本
	if (SysTick_Config(SystemCoreClock / 1000)) // ST3.5.0库版本
	{
		/* Capture error */
		while (1);
	}

	// 关闭滴答定时器
	// SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
}

void IncSysTickCounter(void)
{
	systemTickCounter ++;
}

SysTick_t GetSystemTick(void)
{
	return systemTickCounter;
}

void Delay_ms(unsigned int ms)
{
	SysTick_t t = GetSystemTick();
	t += ms;
	while(GetSystemTick() < t){
		SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPONEXIT); // Reset SLEEPONEXIT
        SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPDEEP);   // Clear SLEEPDEEP bit
        __WFI();                                                // Request Wait For Interrupt
	}
}

/*
 * 微秒级延时
 */
void Delay_us(unsigned int us)
{
	uint32_t val, last;
	int32_t tmp = us*(SystemCoreClock/1000000);

	last = SysTick->VAL;
	while(tmp > 0) {
		val = SysTick->VAL;
		if(val <= last)
			tmp -= last - val;
		else
			tmp -= last + SysTick->LOAD - val;
		last = val;
	}
}
