#ifndef I2C_H__
#define I2C_H__

#include <stdint.h>

typedef enum _I2C_Lib_Op_Type
{
    I2C_OP_Write,
    I2C_OP_Read
}I2C_Lib_Op_Type;

void I2C_Lib_WaitForRWDone(void);
void I2C_Lib_Init(void);
void I2C_Lib_MasterRW(I2C_Lib_Op_Type op,uint8_t slave_addr,uint16_t length,uint8_t* data);

#endif
