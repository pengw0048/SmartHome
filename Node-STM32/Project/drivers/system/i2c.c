#include "i2c.h"
#include "common.h"
#include "cpal_i2c.h"

void I2C_Lib_WaitForRWDone(void)
{
    while ((I2C_HOST_DEV.CPAL_State != CPAL_STATE_READY) && (I2C_HOST_DEV.CPAL_State != CPAL_STATE_ERROR) )
    { }
}

void I2C_Lib_MasterRW(I2C_Lib_Op_Type op,uint8_t slave_addr,uint16_t length,uint8_t* data)
{
    static CPAL_TransferTypeDef CPAL_Transfer; //Must be static because I2C transfer is async

    I2C_Lib_WaitForRWDone();
    
    CPAL_Transfer.wNumData = length;
    CPAL_Transfer.pbBuffer = data;
    CPAL_Transfer.wAddr1 = (uint32_t)slave_addr<<1; //device address is high 7-bit
    // CPAL_Transfer.wAddr2 = (uint32_t)reg_addr;

    I2C_HOST_DEV.wCPAL_Options = CPAL_OPT_NO_MEM_ADDR;

    if(op == I2C_OP_Write){
        I2C_HOST_DEV.pCPAL_TransferTx = &CPAL_Transfer;
        if(CPAL_I2C_Write(&I2C_HOST_DEV) != CPAL_PASS){
            ERR_MSG("CPAL_I2C_Write Failed");
        }
    }
    else if(op == I2C_OP_Read){
        I2C_HOST_DEV.pCPAL_TransferRx = &CPAL_Transfer;
        if(CPAL_I2C_Read(&I2C_HOST_DEV) != CPAL_PASS){
            ERR_MSG("CPAL_I2C_Read Failed");
        }
    }

}

void I2C_Lib_Init()
{
    static bool initialized = false;
    if(initialized)
        return;

    CPAL_I2C_StructInit(&I2C_HOST_DEV);

    // I2C_HOST_DEV.CPAL_Dev is already initialized in definition
    I2C_HOST_DEV.CPAL_ProgModel = CPAL_PROGMODEL_INTERRUPT;
    // I2C_HOST_DEV.pCPAL_I2C_Struct->I2C_ClockSpeed = 350000;
    // I2C_HOST_DEV.pCPAL_I2C_Struct->I2C_OwnAddress1 = 0xAA;

    uint32_t ret = CPAL_I2C_Init(&I2C_HOST_DEV);
    if(ret != CPAL_PASS){
        ERR_MSG("CPAL_I2C_Init Failed");
        return;
    }
    initialized = true;
}

void CPAL_I2C_ERR_UserCallback(CPAL_DevTypeDef pDevInstance, uint32_t Device_Error) 
{
    DBG_MSG("Error: %d", Device_Error);
}


