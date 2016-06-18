#include "stm32f10x.h"
#include "common.h"

void RCC_GPIOClockCmd(GPIO_TypeDef* GPIOx, FunctionalState state)
{
	uint32_t rcc;
	switch((uint32_t)GPIOx) {
		case (uint32_t)GPIOA:
			rcc = RCC_APB2Periph_GPIOA;
			break;
		case (uint32_t)GPIOB:
			rcc = RCC_APB2Periph_GPIOB;
			break;
		case (uint32_t)GPIOC:
			rcc = RCC_APB2Periph_GPIOC;
			break;
		case (uint32_t)GPIOD:
			rcc = RCC_APB2Periph_GPIOD;
			break;
		case (uint32_t)GPIOE:
			rcc = RCC_APB2Periph_GPIOE;
			break;
		default:
			/* Invalid argument! */
			return;
	}
	RCC_APB2PeriphClockCmd(rcc, state);
}

void RCC_USARTClockCmd(USART_TypeDef* USARTx, FunctionalState state)
{
	switch((uint32_t)USARTx) {
		case (uint32_t)USART1:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, state);
			break;
		case (uint32_t)USART2:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, state);
			break;
		case (uint32_t)USART3:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, state);
			break;
		default:
			/* Invalid argument! */
			return;
	}
}

void RCC_TIMClockCmd(TIM_TypeDef* TIMx, FunctionalState NewState)
{
	switch((uint32_t)TIMx) {
		case (uint32_t)TIM2:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, NewState);
			break;
		case (uint32_t)TIM3:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, NewState);
			break;
		case (uint32_t)TIM4:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, NewState);
			break;
		case (uint32_t)TIM5:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, NewState);
			break;
		case (uint32_t)TIM6:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, NewState);
			break;
		case (uint32_t)TIM7:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, NewState);
			break;
		case (uint32_t)TIM12:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, NewState);
			break;
		case (uint32_t)TIM13:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, NewState);
			break;
		case (uint32_t)TIM14:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, NewState);
			break;
		case (uint32_t)TIM1:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, NewState);
			break;
		case (uint32_t)TIM8:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, NewState);
			break;
		case (uint32_t)TIM9:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, NewState);
			break;
		case (uint32_t)TIM10:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, NewState);
			break;
		case (uint32_t)TIM11:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11, NewState);
			break;
		case (uint32_t)TIM15:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, NewState);
			break;
		case (uint32_t)TIM16:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, NewState);
			break;
		case (uint32_t)TIM17:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, NewState);
			break;
		default:
			/* Invalid argument! */
			return;
	}
}

//获得芯片的唯一ID
void Chip_GetUniqueID(uint32_t ChipUniqueID[3])
{
	ChipUniqueID[0] = *(uint32_t *)(0X1FFFF7F0);
    ChipUniqueID[1] = *(uint32_t *)(0X1FFFF7EC);
    ChipUniqueID[2] = *(uint32_t *)(0X1FFFF7E8);
}

//根据频率(Hz)计算TIM的参数
void Timer_16bit_Calc(int freq, uint16_t *period, uint16_t *prescaler)
{
	uint32_t tmp = SystemCoreClock / freq;
	*prescaler = 1;
	while(tmp > 0xffff){
		*prescaler <<= 1;
		tmp >>= 1;
	}
	(*prescaler) --;
	*period = tmp-1;
}