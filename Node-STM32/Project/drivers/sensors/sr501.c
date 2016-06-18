#include "common.h"
#include "sr501.h"
#include "led.h"


void SR501_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_GPIOClockCmd(SR501_PORT, ENABLE);

    GPIO_InitStructure.GPIO_Pin = SR501_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(SR501_PORT, &GPIO_InitStructure);
}

bool SR501_Read(void)
{
    bool val = GPIO_ReadInputDataBit(SR501_PORT, SR501_PIN) == SET;
    LED_BLUE(val);

    return val;
}