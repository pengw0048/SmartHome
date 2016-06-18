#include "common.h"
#include "pwm.h"

typedef void (*TIM_OCxInit_t)(TIM_TypeDef* , TIM_OCInitTypeDef*);
const TIM_OCxInit_t TIM_OCxInit[4] = {TIM_OC1Init, TIM_OC2Init, TIM_OC3Init, TIM_OC4Init};
static uint32_t currentPeriod;

static void PWM_Output_Config(bool bConnectToPWM) 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_GPIOClockCmd(PWM_OUT_PORT, ENABLE); 

    GPIO_InitStructure.GPIO_Pin = PWM_OUT_PIN;
    if(bConnectToPWM)
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    else
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(PWM_OUT_PORT, &GPIO_InitStructure);
    if(!bConnectToPWM){
        GPIO_ResetBits(PWM_OUT_PORT, GPIO_InitStructure.GPIO_Pin);
    }
}

static void TIMx_Config(TIM_TypeDef* TIMx, uint16_t period, uint16_t prescaler)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    RCC_TIMClockCmd(TIMx, ENABLE); 

    /* Time base configuration */        
    TIM_TimeBaseStructure.TIM_Period = period;
    TIM_TimeBaseStructure.TIM_Prescaler = prescaler;            //设置预分频

    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);

    TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Enable);

    TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Enable);

    TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Enable);

    TIM_OC4PreloadConfig(TIMx, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIMx, ENABLE);

    TIM_ITConfig(TIMx, TIM_IT_Update, DISABLE);

}

static void TIMx_OCx_Config(TIM_TypeDef* TIMx, int OCx, uint16_t pulse, uint16_t output)
{
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;       //配置为PWM模式1
    TIM_OCInitStructure.TIM_OutputState = output;   
    TIM_OCInitStructure.TIM_Pulse = pulse;      //设置跳变值，当计数器计数到这个值时，电平发生跳变
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;  //当定时器计数值小于CCR1_Val时为高电平

    TIM_OCxInit[OCx - 1](TIMx, &TIM_OCInitStructure);
    // DBG_MSG("OCx=%d pulse=%d output=%d", OCx, pulse, output);
}

static void TIMx_Enable(TIM_TypeDef* TIMx)
{
    TIM_Cmd(TIMx, ENABLE);
}

static void TIMx_Disable(TIM_TypeDef* TIMx)
{
    TIM_Cmd(TIMx, DISABLE);
}

/*
 * 初始化PWM模块，设定脉冲频率(Hz)
 */
void PWM_Init(int freq)
{
    uint16_t period, prescaler;

    Timer_16bit_Calc(freq, &period, &prescaler);

    TIMx_Config(PWM_OUT_TIM, period, prescaler);
    currentPeriod = period;

    for(int i=1; i<=4; i++)
        PWM_Channel(i, 50, false);

    TIMx_Enable(PWM_OUT_TIM);

    // TIM2_SetInterrupt();
}

/*
 * 设置指定PWM通道的占空比(单位0.01%)和输出状态
 */
void PWM_Channel_Precise(int channel, int percent_100, uint8_t bEnabled)
{
    int pulse = ((currentPeriod+1)*percent_100/10000);
    if(pulse < 0)
        pulse = 0;
    TIMx_OCx_Config(PWM_OUT_TIM, channel, pulse, (bEnabled ? ENABLE : DISABLE));
    PWM_Output_Config(bEnabled);
}

/*
 * 设置指定PWM通道的占空比(%)和输出状态
 */
void PWM_Channel(int channel, int percent, uint8_t bEnabled)
{
    PWM_Channel_Precise(channel, percent*100, bEnabled);
}
