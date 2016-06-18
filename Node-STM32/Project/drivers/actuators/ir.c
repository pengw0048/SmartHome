#include "common.h"
#include "ir.h"
#include "irda.h"
#include "eeprom.h"

#define IR_DATA_BYTES     112
#define TOTAL_IR_CHANNELS 18
static const uint16_t volatile __attribute__((section(".eeprom")))
    irData[TOTAL_IR_CHANNELS][IR_DATA_BYTES/sizeof(uint16_t)];
static czx_vu8 irda_data[IR_DATA_BYTES];

void test_ir()
{
    DBG_MSG("Testing...");
    for (int i = 0; i < IR_DATA_BYTES; ++i)
    {
        irda_data[i] = i;
    }

    FlashEEP_WriteHalfWords(
        (uint16_t*)irda_data,
        sizeof(irda_data)/sizeof(uint16_t),
        (uint32_t)irData[0]
    );
    FlashEEP_WriteHalfWords(
        (uint16_t*)irda_data,
        3,
        (uint32_t)&irData[0][6]
    );
    FlashEEP_WriteHalfWords(
        (uint16_t*)irda_data,
        sizeof(irda_data)/sizeof(uint16_t),
        (uint32_t)irData[9]
    );

    for (int i = 0; i < IR_DATA_BYTES; ++i)
    {
        printf("%d,",irData[0][i]);
    }
    for (int i = 0; i < IR_DATA_BYTES; ++i)
    {
        printf("%d,",irData[9][i]);
    }
    printf("\r\n");
}

void IR_Init()
{
    IRDA_INIT();
}

bool IR_StartLearning(uint8_t channel)
{
    DBG_MSG("Learning channel %d", channel);
    if(channel >= TOTAL_IR_CHANNELS)
        return false;

    IRDA_learn_start();
    while(!IRDA_BUSY_S()){
        LED_BLUE(false);
        Delay_ms(200);
        LED_BLUE(true);
        Delay_ms(200);
    }
    u8 len = IRDA_learn_data_in_out(irda_data);
    DBG_MSG("Learned, length=%d", len);
    if(!len)
        return false;

    FlashEEP_WriteHalfWords(
        (uint16_t*)irda_data,
        sizeof(irda_data)/sizeof(uint16_t),
        (uint32_t)irData[channel]
        );

    return true;
}

bool IR_Emit(uint8_t channel)
{
    DBG_MSG("Emit channel %d", channel);
    if(channel >= TOTAL_IR_CHANNELS)
        return false;

    IRDA_tx_data((czx_vu8*)irData[channel],sizeof(irData[channel]));
    DBG_MSG("Emitted");

    return true;
}

