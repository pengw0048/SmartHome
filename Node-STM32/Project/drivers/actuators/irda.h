/*****************************************************
*
*制作人：
*
* 单位:    深圳市宏芯达科技有限公司
*
* 部门:   技术部
*
* 目的：  STM32与HXD019通讯
*
* 日期:   2013年夏
******************************************************/

#ifndef __IRDA_H__
#define __IRDA_H__
#include "irdaDataType.h"
#include "pin.h"
#include "common.h"


#define     IRDA_NO_ERROR   0

#define     IRDA_DELAY_40US     4
#define     IRDA_DELAY_20MS     20

#define     IRDA_SCL_H()    GPIO_SetBits(IRDA_PORT_IO,IRDA_SCL_PIN)
#define     IRDA_SCL_L()    GPIO_ResetBits(IRDA_PORT_IO,IRDA_SCL_PIN)
#define     IRDA_SDA_H()    GPIO_SetBits(IRDA_PORT_IO,IRDA_SDA_PIN)
#define     IRDA_SDA_L()    GPIO_ResetBits(IRDA_PORT_IO,IRDA_SDA_PIN)
#define     IRDA_GET_ACK()  GPIO_ReadInputDataBit(IRDA_PORT_IO,IRDA_SDA_PIN)
#define     IRDA_GET_DATA() GPIO_ReadInputDataBit(IRDA_PORT_IO,IRDA_SDA_PIN)
#define     IRDA_BUSY_H()   GPIO_SetBits(IRDA_PORT_BUSY,IRDA_BUSY_PIN)
#define     IRDA_BUSY_L()   GPIO_ResetBits(IRDA_PORT_BUSY,IRDA_BUSY_PIN)
#define     IRDA_BUSY_S()   GPIO_ReadInputDataBit(IRDA_PORT_BUSY,IRDA_BUSY_PIN)

#define         IRDA_LED_OFF()  GPIO_SetBits(GPIOB,GPIO_Pin_0)
#define         IRDA_LED_ON()   GPIO_ResetBits(GPIOB,GPIO_Pin_0)


void    IRDA_INIT();
void    IRDA_delay_10us(czx_vu32 t);
void    IRDA_delay_1ms(czx_vu32 t);
void    IRDA_SET_SDA_OUT(void);
void    IRDA_SET_SDA_IN(void);
void    IRDA_SET_SCL_OUT(void);
void    IRDA_SET_BUSY_IN(void);
void    IRDA_SET_BUSY_OUT(void);
czx_u8  IRDA_getACKsign(void);
void    IRDA_sendACKsign(void);

czx_u16 IRDA_open(void);
#define IRDA_close()  IRDA_open()

czx_u16 IRDA_start(void);
czx_u16 IRDA_stop(void);
czx_u16 IRDA_write(czx_vu8 d);
void    IRDA_tx_data(czx_vu8 *d, czx_vu8 len); //发送数据
czx_u16 IRDA_read(czx_vu8 *d);
void    IRDA_learn_start(void);
czx_u8  IRDA_rx_data(czx_vu8 *d);
czx_u8  IRDA_learn_data_in_out(czx_vu8*learn_data_out);


//czx_vu8       IRDA_learn_data_in_out(czx_vu8  *pd);//学习遥控
#endif // __IRDA_H__
