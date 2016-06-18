#include "led.h"
#include "common.h"

void LED_Config()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_GPIOClockCmd(LED_BRD_PORT, ENABLE);

    GPIO_InitStructure.GPIO_Pin = LED_BRD_B_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LED_BRD_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = LED_BRD_G_PIN;
    GPIO_Init(LED_BRD_PORT, &GPIO_InitStructure);

}

void LED_BLUE(bool bOn)
{
    if (bOn)
        GPIO_SetBits(LED_BRD_PORT, LED_BRD_B_PIN);
    else
        GPIO_ResetBits(LED_BRD_PORT, LED_BRD_B_PIN);
}

void LED_GREEN(bool bOn)
{
    if (bOn)
        GPIO_SetBits(LED_BRD_PORT, LED_BRD_G_PIN);
    else
        GPIO_ResetBits(LED_BRD_PORT, LED_BRD_G_PIN);
}
