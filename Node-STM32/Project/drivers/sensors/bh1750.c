/*
This is a library for the BH1750FVI Digital Light Sensor
breakout board.
The board uses I2C for communication. 2 pins are required to
interface to the device.
Written by Christopher Laws, March, 2013.
Ported to STM32 by YuXiang Zhang, Jan 2015.
*/

#include "bh1750.h"
#include "common.h"
#include "i2c.h"


void BH1750_Init(void)
{
    I2C_Lib_Init();
    BH1750_SetMode(BH1750_CONTINUOUS_HIGH_RES_MODE);
}

void BH1750_SetMode(uint8_t mode)
{
    uint8_t wbuf[1] = {mode};
#if BH1750_DEBUG == 1
    DBG_MSG("Setting mode to %d", mode);
#endif
    switch (mode) {
    case BH1750_CONTINUOUS_HIGH_RES_MODE:
    case BH1750_CONTINUOUS_HIGH_RES_MODE_2:
    case BH1750_CONTINUOUS_LOW_RES_MODE:
    case BH1750_ONE_TIME_HIGH_RES_MODE:
    case BH1750_ONE_TIME_HIGH_RES_MODE_2:
    case BH1750_ONE_TIME_LOW_RES_MODE:
        // apply a valid mode change
        // I2C_Lib_WriteByte(BH1750_I2CADDR, mode, 0);
        I2C_Lib_MasterRW(I2C_OP_Write, BH1750_I2CADDR, 1, wbuf);
        I2C_Lib_WaitForRWDone();
        break;
    default:
        // Invalid measurement mode
        ERR_MSG("Invalid measurement mode");
        break;
    }
}


uint16_t BH1750_ReadLightLevel(void)
{
    uint8_t buf[2];
    uint16_t level;

    I2C_Lib_MasterRW(I2C_OP_Read, BH1750_I2CADDR, 2, buf);
    I2C_Lib_WaitForRWDone();

    level = buf[0];
    level <<= 8;
    level |= buf[1];

#if BH1750_DEBUG == 1
    DBG_MSG("Raw light level: %d", level);
#endif

    level = level / 1.2; // convert to lux

#if BH1750_DEBUG == 1
    DBG_MSG("Light level: %d", level);
#endif
    return level;
}

