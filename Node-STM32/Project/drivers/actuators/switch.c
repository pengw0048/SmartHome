#include "common.h"
#include "switch.h"

static  GPIO_TypeDef * const switchPorts[] = RELAY_PORTS;
static const uint16_t      switchPins[] = RELAY_PINS;

STATIC_ASSERT(ARRAY_SIZE(switchPorts) != 0, RELAY_PORTS_not_empty);
STATIC_ASSERT(ARRAY_SIZE(switchPorts) == ARRAY_SIZE(switchPins), Count_of_RELAY_PORTS_matchs_RELAY_PINS);

void Switch_Config()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

    for (int i = 0; i < ARRAY_SIZE(switchPorts); ++i)
    {
        RCC_GPIOClockCmd(switchPorts[i], ENABLE);

        GPIO_InitStructure.GPIO_Pin = switchPins[i];
        GPIO_Init(switchPorts[i], &GPIO_InitStructure);
    }

}

void Switch_Action(bool isON)
{
    GPIO_WriteBit(switchPorts[0], switchPins[0], 
        isON ? RELAY_ACTIVE_VAL : (RELAY_ACTIVE_VAL^1));
    DBG_MSG("isON: %u", isON);
}
void Switch_ChannelAction(int channel, bool isON)
{
    if(channel <= 0 || channel > ARRAY_SIZE(switchPorts)){
        ERR_MSG("channel out of bounds");
        return;
    }
    channel--;
    GPIO_WriteBit(switchPorts[channel], switchPins[channel], 
        isON ? RELAY_ACTIVE_VAL : (RELAY_ACTIVE_VAL^1));
    DBG_MSG("isON[%d]: %u", channel, isON);
}


