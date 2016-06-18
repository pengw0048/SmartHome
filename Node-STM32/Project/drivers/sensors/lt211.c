#include "lt211.h"
#include "usart.h"
#include "systick.h"

static uint16_t crc;

// Compute the MODBUS RTU CRC
static void updateCRC(uint8_t byte)
{
 
    crc ^= (uint16_t)byte;          // XOR byte into least sig. byte of crc
 
    for (int i = 8; i != 0; i--) {    // Loop over each bit
        if ((crc & 0x0001) != 0) {      // If the LSB is set
            crc >>= 1;                    // Shift right and XOR 0xA001
            crc ^= 0xA001;
        }
        else                            // Else LSB is not set
            crc >>= 1;                    // Just shift right
    }
  // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
}

static void LT211_sendbyte_CRC(uint8_t byte)
{
    USART_putchar(LT211_USART, byte);
    updateCRC(byte);
}

static void LT211_sendCRC()
{
    USART_putchar(LT211_USART, (crc>>8)&0xff);
    USART_putchar(LT211_USART, crc&0xff);
}

static void LT211_resetCRC(void)
{
    crc = 0xFFFF;
}

static int8_t LT211_receive(uint8_t *buf, int len, uint16_t timeout)
{
  int read_bytes = 0;
  int ret;
  SysTick_t start_millis;
  
  while (read_bytes < len) {
    start_millis = GetSystemTick();
    ret = -1;
    do {
      if (USART_available(LT211_USART)) {
        ret = USART_getchar(LT211_USART);
        break;
      }
    } while((timeout == 0) || ((GetSystemTick()- start_millis ) < timeout));
    
    if (ret < 0) {
        if(read_bytes){
            return read_bytes;
        }else{
            return -1;
        }
    }
    buf[read_bytes] = (uint8_t)ret;
    // DMSG_HEX(ret);
    read_bytes++;
  }
  return read_bytes;
}

static bool LT211_readRegister16(uint16_t reg, uint16_t* value)
{
    uint8_t buf[7];
    LT211_resetCRC();
    GPIO_SetBits(LT211_TXE_PORT, LT211_TXE_PIN);
    Delay_us(1000);
    LT211_sendbyte_CRC(LT211_ADDR);
    LT211_sendbyte_CRC(3); //Func code: Read Holding Registers
    LT211_sendbyte_CRC(reg>>8);
    LT211_sendbyte_CRC(reg);
    LT211_sendbyte_CRC(0);
    LT211_sendbyte_CRC(1);
    LT211_sendCRC();
    Delay_us(1000);
    GPIO_ResetBits(LT211_TXE_PORT, LT211_TXE_PIN);
    if(LT211_receive(buf, 7, 500) < 7){
        ERR_MSG("Timeout");
        return false;
    }
    if(buf[0] != LT211_ADDR || buf[1] != 3 || buf[2] != 2){
        ERR_MSG("Invalid response");
        return false;
    }
    LT211_resetCRC();
    for (int i = 0; i < 7-2; ++i)
    {
        updateCRC(buf[i]);
    }
    if(crc != ((uint16_t)buf[5]<<8 | buf[6])){
        ERR_MSG("Wrong CRC");
        return false;
    }
    *value = ((uint16_t)buf[3]<<8 | buf[4]);
    return true;
}

bool LT211_Init(void)
{
    uint16_t id;
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_GPIOClockCmd(LT211_TXE_PORT, ENABLE);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = LT211_TXE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LT211_TXE_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(LT211_TXE_PORT, LT211_TXE_PIN);
    USARTx_Config(LT211_USART, LT211_BAUD);
    if(!LT211_readRegister16(0x00, &id))
        return false;
    DBG_MSG("LT211 model %d", id);
    return id == 9000;
}

bool LT211_ReadValueF(float* value, uint8_t type)
{
    uint16_t tmp, tmp2;
    switch(type){
        case LT211_VAL_VOLT:
            if(!LT211_readRegister16(0x48, &tmp))
                return false;
            *value = tmp/100.0;
            break;
        case LT211_VAL_AMP:
            if(!LT211_readRegister16(0x49, &tmp))
                return false;
            *value = tmp/1000.0;
            break;
        case LT211_VAL_WATT:
            if(!LT211_readRegister16(0x4A, &tmp))
                return false;
            *value = tmp;
            break;
        case LT211_VAL_KW_H:
            if(!LT211_readRegister16(0x4B, &tmp))
                return false;
            if(!LT211_readRegister16(0x4C, &tmp2))
                return false;
            *value = ((uint32_t)tmp<<16 | tmp2)/3200.0;
            break;
        default:
            ERR_MSG("Unknown type");
            return false;
    }
    return true;
}
