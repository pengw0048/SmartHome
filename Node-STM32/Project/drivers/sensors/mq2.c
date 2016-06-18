#include "mq2.h"
#include "analog.h"

void MQ2_Init()
{
    Analog_SetChannel(MQ2_ADC_CHANNEL, true);
}

int MQ2_Read()
{
    return Analog_GetChannelValue(MQ2_ADC_CHANNEL);
}