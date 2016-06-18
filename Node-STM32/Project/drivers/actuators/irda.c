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

#include "systick.h"
#include "irda.h"
#define commond0f0h         0xf0
#define commond0f1h         0xf1
#define commond0f2h         0xf2
#define csumcodeformat            0x8



czx_u8      IRDA_get_remote_study_data(czx_vu8 *cmd_data);
czx_u8      IRDA_send_remote_study_data(czx_vu8 *cmd_data, czx_vu16 cmd_lnth);
czx_u32 IRDA_keytogglebit(czx_vu32 *wavedat, czx_vu32 *indatsum);
czx_u32 IRDA_JudgeToggleBit(czx_vu32    *wavedat, czx_vu32  *indatsum);
czx_u32 IRDA_acmpbandc(czx_vu32 cmpdat1, czx_vu32   cmpdat2, czx_vu32   cmpdat3);
czx_u8      IRDA_cmpaequbtog(czx_vu32   cmpdat1, czx_vu32   cmpdat2, czx_vu32   flagw);
void        IRDA_changetogglebit(czx_vu32 *wavedat, czx_vu32    *indatsum, czx_vu32 togtmp9);
czx_u8      IRDA_compdata(czx_vu32  *wavedat, czx_vu32  *indatsum);
czx_u8      IRDA_Modifywave(czx_vu32    *wavedat, czx_vu32  *indatsum);
void        IRDA_delfeng(czx_vu32   *wavedat, czx_vu32  *indatsum);
czx_u8      IRDA_modifywavem708(czx_vu32    *wavedat, czx_vu32  *indatsum);
czx_u8      IRDA_getfigure(czx_vu32 * wavedat, czx_vu32 * indatsum);
czx_u8      IRDA_judgesame(czx_vu32 * dat1, czx_vu32 * dat2, czx_vu32 sum, czx_vu32 flag);
czx_u8      IRDA_cmpdata(czx_vu32   cmpdat1, czx_vu32   cmpdat2, czx_vu32   flag);
void        IRDA_Some_Data_Right_Move1(czx_vu32 *wavedat, czx_vu32  *indatsum, czx_vu32 tmpx);
czx_u32         IRDA_All_Send_Data_Right_Move(czx_vu32  *wavedat, czx_vu32  *indatsum, czx_vu32 tmpx);
void        IRDA_Reload_Data_Buff(czx_vu32 inhb, czx_vu32 inlb, czx_vu32 *wavedat);
czx_u8          g_i2c_cmd_buffer[112] = "";

//wjs;========================
czx_vu32    cnty, cntx;
unsigned char const STandantdata[16] = {
    0xD4, 0x01, 0x38, 0x01, 0xD3, 0x01, 0x36, 0x01,
    0x22, 0x01, 0x83, 0x00, 0xB7, 0x01, 0x37, 0x01
};

//wjs;===============
unsigned char const ToggleBit_Place[7] = {
    0x06, 0x04, 0x0A, 0x06, 0x10, 0x04, 0x02
};
//wjs;=============
unsigned char const code_format[csumcodeformat][87] = {
    {
        0x00, 0x00, 0x28, 0x00, 0xFA, 0x00,
        0x77, 0x01, 0x00, 0x00, 0x28, 0x00, 0x96, 0x01,
        0x13, 0x02, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x0C,
        0x00, 0xF0, 0xA8, 0x00, 0x25, 0x01, 0xF0, 0x00,
        0x00, 0x0C, 0x00, 0xF0, 0xA8, 0x00, 0x25, 0x01,
        0xF1, 0x02, 0xF1, 0x08, 0xF2,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    },
    {
        0x00, 0x00, 0x28,
        0x00, 0xD4, 0x00, 0x64, 0x01, 0x00, 0x00, 0x28,
        0x00, 0x70, 0x01, 0x1F, 0x02, 0x00, 0x00, 0xF1,
        0x05, 0xF1, 0x06, 0xF2,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {
        0x16,
        0x00, 0x23, 0x00, 0x3E, 0x00, 0xBB, 0x00, 0x16,
        0x00, 0x23, 0x00, 0xDA, 0x00, 0x57, 0x01, 0x00,
        0x00, 0xF1, 0x05, 0xF1, 0x07, 0xF2,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {
        0x01, 0x00, 0x37, 0x00,
        0xDA, 0x00, 0x70, 0x01, 0x00, 0x00, 0x37, 0x00,
        0x83, 0x01, 0x13, 0x02, 0x00, 0x00, 0xF0, 0x01,
        0x00, 0x37, 0x00, 0xF0, 0x8C, 0x00, 0x28, 0x01,
        0xF0, 0x01, 0x00, 0x37, 0x00, 0xF0, 0x8C, 0x00,
        0x28, 0x01, 0xF1, 0x04, 0xF1, 0x06, 0xF2,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    },
    {
        0x12, 0x00,
        0x24, 0x00, 0x12, 0x00, 0x24, 0x00, 0x44, 0x00,
        0x1B, 0x00, 0x2B, 0x00, 0x1B, 0x00, 0x01, 0x00,
        0xF0, 0x6A, 0x00, 0xE7, 0x00, 0xF0, 0x30, 0x00,
        0x3D, 0x00, 0xF0, 0x12, 0x00, 0x24, 0x00, 0xF0,
        0x12, 0x00, 0x24, 0x00, 0xF0, 0x12, 0x00, 0x24,
        0x00, 0xF0, 0x12, 0x00, 0x24, 0x00, 0xF0, 0x12,
        0x00, 0x24, 0x00, 0xF0, 0x12, 0x00, 0x24, 0x00,
        0xF0, 0x12, 0x00, 0x24, 0x00, 0xF0, 0x12, 0x00,
        0x24, 0x00, 0xF0, 0x31, 0x00, 0x3D, 0x00, 0xF0,
        0x31, 0x00, 0x3D, 0x00, 0xF1, 0x08, 0xF1, 0x08,
        0xF1, 0x08, 0xF1, 0x08, 0xF2
    },
    {
        0x12, 0x00, 0x24,
        0x00, 0x12, 0x00, 0x24, 0x00, 0x5A, 0x00, 0x37,
        0x00, 0x3E, 0x00, 0x1B, 0x00, 0x01, 0x00, 0xF0,
        0x6A, 0x00, 0xE7, 0x00, 0xF0, 0x31, 0x00, 0x3D,
        0x00, 0xF1, 0x04, 0xF0, 0x31, 0x00, 0x3D, 0x00,
        0xF0, 0x31, 0x00, 0x3D, 0x00, 0xF1, 0x08, 0xF1,
        0x08, 0xF2,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00
    },
    {
        0x28,
        0x00, 0x41, 0x00, 0x28, 0x00, 0x41, 0x00, 0x76,
        0x00, 0x34, 0x00, 0x5D, 0x00, 0x34, 0x00, 0x01,
        0x01, 0xF1, 0x08, 0xF1, 0x06, 0xF2,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {
        0x12, 0x00, 0x2B, 0x00, 0x12, 0x00, 0x2B, 0x00,
        0x12, 0x00, 0x2B, 0x00, 0x3F, 0x00, 0x1F, 0x00,
        0x01, 0x00, 0xF0, 0x12, 0x00, 0x2B, 0x00, 0xF0,
        0x76, 0x00, 0xC8, 0x00, 0xF0, 0x12, 0x00, 0x2B,
        0x00, 0xF0, 0x2F, 0x00, 0x5D, 0x00, 0xF0, 0x2F,
        0x00, 0x5D, 0x00, 0xF0, 0x12, 0x00, 0x2B, 0x00,
        0xF1, 0x0D, 0xF0, 0x12, 0x00, 0x2B, 0x00, 0xF0,
        0x38, 0x01, 0x23, 0x0F, 0xF0, 0x12, 0x00, 0x2B,
        0x00, 0xF0, 0x76, 0x00, 0xC8, 0x00, 0xF2,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
};


GPIO_InitTypeDef IRDA_InitStructure;


void IRDA_delay_10us(czx_vu32 t) //延时10us
{
    Delay_us(t * 10);
}
void IRDA_delay_1ms(czx_vu32 t) //延时1ms
{
    Delay_ms(t);
}


void    IRDA_SET_SDA_IN(void)
{
    IRDA_InitStructure.GPIO_Pin = IRDA_SDA_PIN;
    IRDA_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    IRDA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IRDA_PORT_IO, &IRDA_InitStructure);
}
void    IRDA_SET_SDA_OUT(void)
{
    IRDA_InitStructure.GPIO_Pin = IRDA_SDA_PIN;
    IRDA_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    IRDA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IRDA_PORT_IO, &IRDA_InitStructure);
}
void    IRDA_SET_SCL_OUT(void)
{
    IRDA_InitStructure.GPIO_Pin = IRDA_SCL_PIN;
    IRDA_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    IRDA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IRDA_PORT_IO, &IRDA_InitStructure);
}
void    IRDA_SET_BUSY_IN(void)
{
    IRDA_InitStructure.GPIO_Pin = IRDA_BUSY_PIN;
    IRDA_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    IRDA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IRDA_PORT_BUSY, &IRDA_InitStructure);
}
void    IRDA_SET_BUSY_OUT(void)
{
    IRDA_InitStructure.GPIO_Pin = IRDA_BUSY_PIN;
    IRDA_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    IRDA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IRDA_PORT_BUSY, &IRDA_InitStructure);
}
void    IRDA_INIT() //上电初始化
{
    RCC_GPIOClockCmd(IRDA_PORT_BUSY, ENABLE);
    RCC_GPIOClockCmd(IRDA_PORT_IO, ENABLE);

    IRDA_SDA_H();
    IRDA_SET_SDA_OUT();

    IRDA_SCL_H();
    IRDA_SET_SCL_OUT();
    IRDA_SET_BUSY_IN();
}

czx_u8  IRDA_getACKsign(void)
{
    czx_vu8 ack;
    IRDA_SET_SDA_IN();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SCL_H();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    ack =   IRDA_GET_DATA();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SCL_L();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    return ack;
}
void    IRDA_sendACKsign(void)
{
    IRDA_SET_SDA_OUT();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SDA_L();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SCL_H();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SCL_L();
}
czx_u16 IRDA_open(void)
{
    IRDA_SET_SDA_OUT();
    IRDA_SET_SCL_OUT();
    IRDA_SCL_H();
    IRDA_SDA_H();
    return IRDA_NO_ERROR;
}

czx_u16 IRDA_start(void)
{
    IRDA_SET_SDA_OUT();
    IRDA_SET_SCL_OUT();
    IRDA_SCL_H();
    IRDA_SDA_H();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SDA_L();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SCL_L();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    return TRUE;
}
czx_u16 IRDA_stop(void)
{
    IRDA_SET_SDA_OUT();
    IRDA_SET_SCL_OUT();
    IRDA_SCL_L();
    IRDA_SDA_L();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SCL_H();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SDA_H();
    IRDA_delay_10us(IRDA_DELAY_40US);
    return IRDA_NO_ERROR;
}
czx_u16 IRDA_write(czx_vu8 d)
{
    czx_vu8 d_bit;
    //czx_vu8 i;
    signed char i ;
    IRDA_delay_10us(IRDA_DELAY_40US);
    for (i = 7; i >= 0; i--) {
        IRDA_delay_10us(IRDA_DELAY_40US);

        d_bit = ((d >> i) & 0x01);

        if (d_bit) {
            IRDA_SDA_H();
        } else {
            IRDA_SDA_L();
        }
        IRDA_delay_10us(IRDA_DELAY_40US);
        IRDA_SCL_H();
        IRDA_delay_10us(IRDA_DELAY_40US);
        IRDA_SCL_L();
    }
    return IRDA_getACKsign();
}
void    IRDA_tx_data(czx_vu8 *d, czx_vu8 len)
{
    czx_vu8 i;
    IRDA_open();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SCL_L();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SCL_H();
    IRDA_delay_1ms(IRDA_DELAY_20MS);
    IRDA_start();
    IRDA_delay_10us(IRDA_DELAY_40US);
    for (i = 0; i < len; i++) {
        IRDA_write(d[i]);
        IRDA_delay_10us(IRDA_DELAY_40US);
    }
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_stop();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_close();
    IRDA_delay_10us(IRDA_DELAY_40US);
}
void        IRDA_learn_start(void) //学习开始
{
    IRDA_open();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SCL_L();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SCL_H();
    IRDA_delay_1ms(IRDA_DELAY_20MS);
    IRDA_start();
    IRDA_delay_10us(IRDA_DELAY_40US);

    IRDA_write(0x30);//写命令
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_write(0x10);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_write(0x40);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_stop();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_close();
    IRDA_delay_10us(IRDA_DELAY_40US);
}
czx_u16 IRDA_read(czx_vu8 *d)
{
    czx_vu8 i   =   8;
    czx_vu8 rd = 0;
    IRDA_SET_SDA_IN();
    while (i--) {
        rd <<= 1;
        IRDA_SCL_H();
        IRDA_delay_10us(IRDA_DELAY_40US);
        rd |= IRDA_GET_DATA();
        IRDA_SCL_L();
        IRDA_delay_10us(IRDA_DELAY_40US);
        IRDA_delay_10us(IRDA_DELAY_40US);
    }
    IRDA_SCL_L();
    IRDA_delay_10us(IRDA_DELAY_40US);
    *d  =   rd;
    IRDA_sendACKsign();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    return IRDA_NO_ERROR;
}
czx_u8      IRDA_rx_data(czx_vu8 *d)
{
    czx_vu8  bv, i = FALSE, check;
    IRDA_open();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SCL_L();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_SCL_H();
    IRDA_delay_1ms(IRDA_DELAY_20MS);
    IRDA_start();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_write(0x30);//写命令
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_write(0x52);
    IRDA_delay_10us(IRDA_DELAY_40US);

    IRDA_start();   //读
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_write(0x31);//写命令
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_read(&bv);
    IRDA_delay_10us(IRDA_DELAY_40US);
    if (bv != FALSE) {
        IRDA_stop();
        IRDA_delay_10us(IRDA_DELAY_40US);
        IRDA_close();
        IRDA_delay_10us(IRDA_DELAY_40US);
        return FALSE;
    }
    i = FALSE;
    d[i]    =   bv;
    check   =   0xb3;
    for (i = 1; i < 110; i++) {
        IRDA_read(&bv);
        IRDA_delay_10us(IRDA_DELAY_40US);
        d[i]    =   bv;
        check += bv;
    }
    IRDA_read(&bv);
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_stop();
    IRDA_delay_10us(IRDA_DELAY_40US);
    IRDA_close();
    IRDA_delay_10us(IRDA_DELAY_40US);
    if (bv != check) {
        return FALSE;
    }
    return TRUE;
}
czx_u8      IRDA_Modify_wave(czx_vu32   *wavedat, czx_vu32  *indatsum)
{
    czx_vu32    lrn_flag, receive_time, repdatalen;
    czx_vu32    waveld1 = 0, waveld2 = 0;
    czx_vu32    wavedd1, wavedd2;
    czx_vu32    cnt1, cnt2, cnt3, cnt4;
    if (wavedat[0] == 0) {
        lrn_flag = wavedat[1];          //wjs;标志变量
        receive_time = wavedat[2];
        repdatalen = wavedat[3];
        cnt3 = 0;       //parity--//receive_time
        cnt1 = 5;       //point-y===cnt2---point-x
        for (cnt2 = 5; cnt2 < *indatsum + 1; cnt2++) {
            wavedd1 = wavedat[cnt2];
            if (wavedd1 >= 0x80) {
                wavedd2 = wavedd1 & 0x7f;
                cnt2++;
                if (cnt2 == *indatsum + 1) {
                    wavedd1 = 0;
                } else {
                    wavedd1 = wavedat[cnt2];
                }
            } else {
                wavedd2 = 0;
            }
            waveld1 = wavedd2 * 0x100;
            waveld1 += wavedd1;
            if ((cnt3 & 0x01) == 0) {
                if (waveld1 > 7) {
                    waveld1 -= 7;
                } else  waveld1 = 2;
            } else {
                waveld1 += 3;
            }
            wavedd2 = waveld1 / 0x100;
            wavedd1 = waveld1 % 0x100;
            if ((wavedd2 > 0) || (wavedd1 >= 0x80)) {
                wavedd2 |= 0x80;
                wavedat[cnt1++] = wavedd2;
                wavedat[cnt1++] = wavedd1;
            } else
                wavedat[cnt1++] = wavedd1;
            cnt3++;
            if (cnt3 >= receive_time) {
                receive_time++;
                break;
            }
        }
        receive_time--;
        cnt4 = 5;
        if (wavedat[5] >= 0x80) {               //wjs;相当于9ms的数据
            cnt4 = 7;
            if (wavedat[6] < 253) {
                wavedat[6] += 3;
            } else {
                wavedat[6] += 3;
                wavedat[6] -= 0x100;
                wavedat[5]++;
            }
        } else {
            cnt4 = 6;
            if (wavedat[5] < 126)
                wavedat[5] += 2;
            else
                wavedat[5] = 127;
        }
        if (wavedat[cnt4] >= 0x80) {
            if (wavedat[cnt4 + 1] != 0) {
                wavedat[cnt4 + 1] -= 1;
            } else {
                wavedat[cnt4 + 1] = 255;
                wavedat[cnt4] &= 0x7f;
                wavedat[cnt4]--;
                wavedat[cnt4] |= 0x80;
            }
        } else {
            if (wavedat[cnt4] < 2) {
                wavedat[cnt4] = 2;
            }
        }
        return  FALSE;
    } else
        return  TRUE;
    //return    FALSE;  //ok
}
void        IRDA_delfeng(czx_vu32   *wavedat, czx_vu32  *indatsum)
{
    czx_vu32    lrn_flag, receive_time, repdatalen;
    czx_vu32    waveld1 = 0, waveld2 = 0;
    czx_vu32    wavedd1, wavedd2, wavedd4 = 0, wavedd5, wavedd6;
    czx_vu32    cnt1, cnt2, cnt3;
    lrn_flag = wavedat[1];
    receive_time = wavedat[2];
    repdatalen = wavedat[3];
    cnt3 = 0;       //parity--//receive_time
    cnt1 = 5;       //point-y===cnt2---point-x
    for (cnt2 = 5; cnt2 < *indatsum + 1; cnt2++) {
        wavedd1 = wavedat[cnt2];
        wavedat[cnt1++] = wavedd1;
        if (wavedd1 >= 0x80) {
            wavedd2 = wavedd1 & 0x7f;
            cnt2++;
            if (cnt2 == *indatsum + 1) {
                wavedd1 = 0;
            } else {
                wavedd1 = wavedat[cnt2];
            }
            wavedat[cnt1++] = wavedd1;
        } else {
            wavedd2 = 0;
        }
        if ((cnt3 & 0x01) == 1) {
            if (wavedd2 == 0) {
                if (wavedd1 < 5) {
                    cnt1--; //point-y
                    if (wavedd4 >= 0x80)
                        cnt1--;
                    cnt1--;
                    wavedd5 = wavedat[cnt1];
                    if (wavedd5 >= 0x80) {
                        wavedd6 = wavedd5 & 0x7f;
                        wavedd5 = wavedat[cnt1 + 1];
                    } else {
                        wavedd6 = 0;
                    }
                    waveld1 = wavedd2 * 0x100;
                    waveld1 += wavedd1;
                    waveld2 = wavedd6 * 0x100;
                    waveld2 += wavedd5;
                    waveld1 += waveld2;
                    wavedd5 = wavedat[cnt2++];
                    if (wavedd5 >= 0x80) {
                        wavedd6 = wavedd5 & 0x7f;
                        wavedd5 = wavedat[cnt2++];
                    } else {
                        wavedd6 = 0;
                    }
                    waveld2 = wavedd6 * 0x100;
                    waveld2 += wavedd5;
                    waveld1 += waveld2;
                    wavedd2 = waveld1 / 0x100;
                    wavedd1 = waveld1 % 0x100;
                    if (wavedd2 >= 0x80) {
                        wavedd2 |= 0x80;
                        wavedat[cnt1++] = wavedd2;
                    }
                    wavedat[cnt1++] = wavedd1;
                    receive_time--;
                    receive_time--;
                    cnt3++;
                }
            }
        }
        wavedd4 = wavedd2;
        cnt3++;
        if (cnt3 >= receive_time) {
            receive_time++;
            break;
        }
    }
    receive_time--;
    return;
}
czx_u8 IRDA_modifywavem708(czx_vu32 *wavedat, czx_vu32  *indatsum)
{
    czx_vu32    lrn_flag, receive_time, repdatalen;
    czx_vu32    waveld1 = 0, waveld2 = 0;
    czx_vu32    wavedd1, wavedd2, wavedd3;
    czx_vu32    cnt1, cnt2, cnt3, cnt4;
    if (wavedat[0] == 0) {
        lrn_flag = wavedat[1];
        receive_time = wavedat[2];
        repdatalen = wavedat[3];
        cnt3 = 0;       //parity--//receive_time
        cnt1 = 5;       //point-y===cnt2---point-x
        for (cnt2 = 5; cnt2 < *indatsum + 1; cnt2++) {
            wavedd1 = wavedat[cnt2];
            if ((wavedd1 & 0xf0) == 0xf0) {
                wavedd3 = wavedd1 & 0x0f;
                cnt2++;
                wavedd2 = wavedat[cnt2];
                cnt2++;
                wavedd1 = wavedat[cnt2];
            } else  if (wavedd1 >= 0x80) {
                wavedd3 = 0;
                wavedd2 = wavedd1 & 0x7f;
                cnt2++;
                wavedd1 = wavedat[cnt2];
            } else {
                wavedd3 = 0;
                wavedd2 = 0;
            }
            waveld1 = wavedd3 * 0x10000;
            waveld1 += wavedd2 * 0x100;
            waveld1 += wavedd1;
            waveld1 += 2;
            waveld1 /= 16;
            if ((cnt3 & 0x01) == 0) {
                if (waveld1 >= 256) {
                    waveld1 -= 1;
                }
                if (waveld1 > 128)
                    return  1;  //载波波型大于2048us--error
            } else {
                waveld1 -= waveld2;
                if (waveld1 <= 2) {
                    waveld1 = 1;
                } else {
                    waveld1 -= 2;
                }
            }
            waveld2 = waveld1;
            wavedd2 = waveld1 / 0x100;
            wavedd1 = waveld1 % 0x100;
            if ((wavedd2 > 0) || (wavedd1 >= 0x80)) {
                wavedd2 |= 0x80;
                wavedat[cnt1++] = wavedd2;
                wavedat[cnt1++] = wavedd1;
            } else
                wavedat[cnt1++] = wavedd1;
            cnt3++;
            if (cnt3 >= receive_time)
                break;
        }
        cnt4 = 5;
        if (wavedat[5] >= 0x80) {
            cnt4 = 7;
            if (wavedat[6] >= 5) {
                wavedat[6] -= 5;
            } else {
                wavedat[6] += 0x100;
                wavedat[6] -= 5;
                wavedat[5] &= 0x7f;
                wavedat[5]--;
                wavedat[5] |= 0x80;
            }
        } else {
            cnt4 = 6;
            if (wavedat[5] > 3)
                wavedat[5] -= 3;
            else    wavedat[5] = 1;
        }
        if (wavedat[cnt4] >= 0x80) {
            if (wavedat[cnt4 + 1] < 252) {
                wavedat[cnt4 + 1] += 4;
            } else {
                wavedat[cnt4 + 1] -= 252;
                wavedat[cnt4] += 1;
            }
        } else {
            if (wavedat[cnt4] < 0x7c) {
                wavedat[cnt4] += 3;
            } else
                wavedat[cnt4] = 0x7f;
        }
        return  0;
    } else
        return  1;
    //return    0;  //ok
}

czx_u8  IRDA_getfigure(czx_vu32 * wavedat, czx_vu32 * indatsum)
{
    czx_vu32    cnt1, cnt2, cnt3, cnt4;
    czx_vu32    maxdat[3], maxdatindex[3], maxwave, maxwave2, maxwindex, waved1;
    czx_vu32    r_figure;
    czx_vu32    lrn_flag, receive_time, repdatalen;
    czx_vu32    len, flag2;
    czx_vu32    *ptdat2, *ptdat1;
    czx_vu32    flag3;
    if (wavedat[0] == 0) {
        cnt3 = 0;
        maxwave = 0;
        for (cnt2 = 5; cnt2 < *indatsum; cnt2++) {
            waved1 = wavedat[cnt2];
            if (waved1 >= 0x80) {
                waved1 = (waved1 - 0x80);
                waved1 = waved1 * 0x100;    //waved1=waved1*0x80;
                cnt2++;
                waved1 += wavedat[cnt2];
            }
            if (maxwave < waved1) {
                maxwave = waved1;
                maxwindex = cnt3;
            }
            cnt3++;
        }
        maxwave2 = maxwave / 8;
        maxwave2 = maxwave - maxwave2;
        cnt1 = 0;
        maxdat[0] = 0;
        maxdatindex[0] = 0;
        maxdat[1] = 0;
        maxdatindex[1] = 0;
        maxdat[2] = 0;
        maxdatindex[2] = 0;
        cnt3 = 0;
        for (cnt2 = 5; cnt2 < *indatsum; cnt2++) {
            cnt4 = cnt2;
            waved1 = wavedat[cnt2];
            if (waved1 >= 0x80) {
                waved1 = (waved1 - 0x80);
                waved1 = waved1 * 0x100;    //waved1=waved1*0x80;
                cnt2++;
                waved1 += wavedat[cnt2];
            }
            if (maxwave >= waved1 && maxwave2 <= waved1) {
                maxdatindex[cnt1] = cnt4;       //wave data index x
                maxdat[cnt1] = cnt3;        //wave number receive_time
                cnt1++;
                if (3 <= cnt1)
                    break;
            }
            cnt3++;
        }
        r_figure = 0;
        lrn_flag = wavedat[1];
        receive_time = wavedat[2];
        repdatalen = wavedat[3];
        if ((lrn_flag & 0x40) != 00) {
            if (maxdat[1] == 0)
                repdatalen = receive_time;
            else {
                if (wavedat[maxdatindex[1]] < 0x88) {
                    repdatalen = receive_time;
                } else {
                    goto    nosinglewave2;
//                  if(maxdat[0]==0)
//                      maxdat[0]=receive_time;
//                  r_figure=1;
//                  receive_time=maxdat[0];
//                  repdatalen=maxdat[0];
                }
            }
        } else {
            if (maxdat[1] == 0) {
                if (maxdat[0] == 0)
                    maxdat[0] = receive_time;
                r_figure = 1;
                receive_time = maxdat[0];
                repdatalen = maxdat[0];
            } else {
nosinglewave2:
                cnt4 = maxdat[1] - maxdat[0];
                if (cnt4 < 2) {
                    if (maxdat[0] == 0)
                        maxdat[0] = receive_time;
                    r_figure = 1;
                    receive_time = maxdat[0];
                    repdatalen = maxdat[0];
                } else  if (cnt4 >= maxdat[0]) {
                    flag2 = lrn_flag & 0x80;
                    ptdat1 = &wavedat[5];
                    if (wavedat[maxdatindex[0]] >= 0x80)
                        ptdat2 = &wavedat[maxdatindex[0] + 2];
                    else
                        ptdat2 = &wavedat[maxdatindex[0] + 1];
                    len = maxdat[0] - 2;
                    if (0 == IRDA_judgesame(ptdat1, ptdat2, len, flag2)) {
                        if (maxdat[0] == 0)
                            maxdat[0] = receive_time;
                        r_figure = 1;
                        receive_time = maxdat[0];
                        repdatalen = maxdat[0];
                    } else {
                        if (wavedat[maxdatindex[1]] >= 0x80)
                            ptdat2 = &wavedat[maxdatindex[1] + 2];
                        else
                            ptdat2 = &wavedat[maxdatindex[1] + 1];
                        ptdat1 = &wavedat[5];
                        len = maxdat[0] - 2;
                        if (0 == IRDA_judgesame(ptdat1, ptdat2, len, flag2)) {
                            r_figure = 3;
                            receive_time = maxdat[0];
                            repdatalen = maxdat[1];
                        } else {
                            r_figure = 2;
                            receive_time = maxdat[0];
                            repdatalen = maxdat[1];
                        }
                    }
                } else {
                    cnt3 = maxdat[0] / 2;
                    if (cnt4 >= cnt3) {
                        r_figure = 2;
                        receive_time = maxdat[0];
                        repdatalen = maxdat[1];
                    } else {
                        //nosinglewave4
                        flag3 = lrn_flag & 0x80;
                        ptdat1 = &wavedat[5];
                        if (wavedat[maxdatindex[0]] >= 0x80)
                            ptdat2 = &wavedat[maxdatindex[0] + 1];
                        else
                            ptdat2 = &wavedat[maxdatindex[0]];
                        len = maxdat[0] - 2;
                        if (0 == IRDA_judgesame(ptdat1, ptdat2, len, flag3)) {
                            if (maxdat[0] == 0)
                                maxdat[0] = receive_time;
                            r_figure = 1;
                            receive_time = maxdat[0];
                            repdatalen = maxdat[0];
                        } else {
                            if (wavedat[maxdatindex[1]] >= 0x80)
                                ptdat2 = &wavedat[maxdatindex[1] + 1];
                            else
                                ptdat2 = &wavedat[maxdatindex[1]];
                            ptdat1 = &wavedat[5];
                            len = maxdat[0] - 2;
                            if (0 == IRDA_judgesame(ptdat1, ptdat2, len, flag3)) {      //wjs;y
                                r_figure = 3;
                                receive_time = maxdat[0];
                                repdatalen = maxdat[1];
                            } else {
                                r_figure = 2;
                                receive_time = maxdat[0];
                                repdatalen = maxdat[1];
                            }
                        }
                    }
                }
            }
        }
        if (receive_time % 2 == 1)
            receive_time++;
        if (repdatalen % 2 == 1)
            repdatalen++;

        r_figure &= 0X07;
        lrn_flag &= 0X88;
        wavedat[1] = r_figure | lrn_flag;
        wavedat[2] = receive_time;
        wavedat[3] = repdatalen;
        return  FALSE;
    } else
        return  TRUE;
    //return    FALSE;
}
czx_u8 IRDA_judgesame(czx_vu32 * dat1, czx_vu32* dat2, czx_vu32 sum, czx_vu32 flag)
{
    unsigned int i, j, k, dd1, dd2;
    j = FALSE;
    k = FALSE;
    for (i = 0; i < sum; i++) {
        dd1 = dat1[k];
        if (dd1 >= 0x80) {
            dd1 = (dd1 - 0x80);
            dd1 = dd1 * 0x100;      //dd1=dd1*0x80;
            k++;
            dd1 += dat1[k];
        }
        k++;
        dd2 = dat2[j];
        if (dd2 >= 0x80) {
            dd2 = (dd2 - 0x80);
            dd2 = dd2 * 0x100;      //dd2=dd2*0x80;
            j++;
            dd2 += dat2[j];
        }
        j++;
        if (IRDA_cmpdata(dd1, dd2, flag))
            return  TRUE;
    }
    return  FALSE;
}
czx_u8 IRDA_cmpdata(czx_vu32    cmpdat1, czx_vu32   cmpdat2, czx_vu32   flag)
{
    czx_vu32    data1, data2, data3;
    if (cmpdat1 >= 32) {
        data1 = cmpdat1 / 8;
        if (cmpdat1 >= cmpdat2)
            data2 = cmpdat1 - cmpdat2;
        else
            data2 = cmpdat2 - cmpdat1;
        if (data1 >= data2)
            return  FALSE;  //ok
        else
            return  TRUE;   //fail
    } else {
        if (cmpdat2 > 128)
            return  TRUE;   //fail
        if (cmpdat1 >= cmpdat2) {
            data1 = cmpdat1 / 4;
            data2 = cmpdat1 - cmpdat2;
            data3 = cmpdat2;
        } else {
            data1 = cmpdat2 / 4;
            data2 = cmpdat2 - cmpdat1;
            data3 = cmpdat1;
        }
        if (data3 < 16) {
            if (0 == flag) {
                if (data3 < 5) {
                    data1 = data1 * 2;
                    if (data3 < data1)
                        return  TRUE;   //fail
                    else
                        return  FALSE;  //ok
                } else {
                    if (data2 < data1)
                        return  FALSE;  //ok
                    else
                        return  TRUE;   //fail
                }
            } else {
                if (data2 < 5)
                    return  FALSE;  //ok
                else
                    return  TRUE;   //fail
            }
        } else {
            if (data2 < data1)
                return  FALSE;  //ok
            else
                return  TRUE;   //fail
        }
    }
    //return    FALSE;
}
void IRDA_Some_Data_Right_Move1(czx_vu32    *wavedat, czx_vu32  *indatsum, czx_vu32 tmpx)
{
    cntx = *indatsum - 1;
    cnty = *indatsum;
    if (cntx < tmpx)
        return;
    while (cntx > tmpx) {
        wavedat[cnty--] = wavedat[cntx--];
    }
    wavedat[cnty] = wavedat[cntx];
    return;
}


czx_u8 IRDA_cmpaequbtog(czx_vu32    cmpdat1, czx_vu32   cmpdat2, czx_vu32   flagw)
{
    czx_vu32    tmpd1, tmpd2;

    if (cmpdat1 < 0x20) {
        if (cmpdat2 >= 0x100)
            return  1;
        if (cmpdat2 >= cmpdat1) {
            tmpd2 = cmpdat2 / 2;
            tmpd2 /= 2;
            tmpd1 = cmpdat2 - cmpdat1;
            cmpdat2 = cmpdat1;
        } else {
            tmpd2 = cmpdat1 / 2;
            tmpd2 /= 2;
            tmpd1 = cmpdat1 - cmpdat2;
        }
        if (cmpdat2 >= 0x10) {
            if (tmpd1 >= tmpd2)
                return  1;
            else
                return  0;
        } else {
            if (0 == flagw) {
                if (cmpdat2 >= 5) {
                    if (tmpd1 >= tmpd2)
                        return  1;
                    else
                        return  0;
                } else
                    return  0;
            } else {
                if (tmpd1 >= 5)
                    return  1;
                else
                    return  0;
            }

        }
    } else {
        tmpd1 = cmpdat1 / 2;
        tmpd1 /= 2;
        tmpd1 /= 2;
        if (cmpdat1 >= cmpdat2)
            tmpd2 = cmpdat1 - cmpdat2;
        else
            tmpd2 = cmpdat2 - cmpdat1;
        if (tmpd2 >= tmpd1)
            return  1;
        else
            return  0;
    }

}
//====================
czx_u32 IRDA_All_Send_Data_Right_Move(czx_vu32  *wavedat, czx_vu32  *indatsum, czx_vu32 tmpx)
{
    czx_vu32    tmplen;
    cntx = *indatsum - 4;
    tmplen = cntx;
    cnty = *indatsum;
    if (tmpx >= cntx) {
        tmpx = cntx - 1;
    }
    while (cntx > tmpx) {
        wavedat[cnty--] = wavedat[cntx--];
    }
    wavedat[cnty] = wavedat[cntx];
    return  tmpx;
}
czx_u32 IRDA_acmpbandc(czx_vu32 cmpdat1, czx_vu32   cmpdat2, czx_vu32   cmpdat3)
{
    //A==0:A在B,C之间;==0F0H:no,>较大的数;==0Fh:no,<较小的数;
    if ((cmpdat1 >= cmpdat2) && (cmpdat3 >= cmpdat1))
        return  0;
    else    if (cmpdat3 < cmpdat1)
        return  0xf0;
    else
        return  0x0f;
}
czx_vu32    cnty, cntx;

czx_u32 IRDA_JudgeToggleBit(czx_vu32    *wavedat, czx_vu32  *indatsum)
{
    czx_vu32    togtmp12 = 0, togtmp13 = 0;
    czx_vu32    togtmp6, togtmp5, togtmp4, togtmp3, togtmp2, togtmp1, togtmp14, togtmp15, togtmp16;
    czx_vu32    cmpa, cmpb, cmpc;
    czx_vu32    cnt1;
    czx_vu32    outcnt1;
    czx_vu8 cmpbcf, r_flag13, progf1;

    //ToggleBit_Judge_Next_For:
    for (outcnt1 = csumcodeformat; outcnt1 != 0; outcnt1--) {
        if (0 == code_format[csumcodeformat - outcnt1][16]) {           //wjs;Y
            togtmp14 = 18;  //TOGTMP14
            cntx = 5;       //---x
            while (1) { //ToggleBit_Judge_One_Format
                togtmp16 = cntx;    //---x--bk--TOGTMP16
                togtmp6 = 0;
                togtmp5 = wavedat[cntx++];
                if (togtmp5 >= 0x80) {
                    togtmp6 = togtmp5;
                    togtmp5 = wavedat[cntx++];
                }
                if (commond0f0h == code_format[csumcodeformat - outcnt1][togtmp14]) {
                    togtmp3 = code_format[csumcodeformat - outcnt1][togtmp14 + 1];
                    togtmp4 = code_format[csumcodeformat - outcnt1][togtmp14 + 2];
                    togtmp1 = code_format[csumcodeformat - outcnt1][togtmp14 + 3];
                    togtmp2 = code_format[csumcodeformat - outcnt1][togtmp14 + 4];
                    togtmp14 += 5;
                    cmpa = togtmp6 & 0x7f;
                    cmpa *= 0x100;
                    cmpa += togtmp5;
                    cmpb = togtmp4 & 0x7f;
                    cmpb *= 0x100;
                    cmpb += togtmp3;
                    cmpc = togtmp2 & 0x7f;
                    cmpc *= 0x100;
                    cmpc += togtmp1;
                    if (0 == IRDA_acmpbandc(cmpa, cmpb, cmpc))
                        continue;
                    else
                        break;
                } else  if (commond0f1h == code_format[csumcodeformat - outcnt1][togtmp14]) {
                    togtmp15 = code_format[csumcodeformat - outcnt1][togtmp14 + 1]; //TOGTMP15
                    togtmp14 += 2;
                    cmpbcf = 0;
                    while (1) { //ToggleBit_Judge_BIT
                        cnt1 = 0;
                        togtmp3 = code_format[csumcodeformat - outcnt1][cnt1++];
                        togtmp4 = code_format[csumcodeformat - outcnt1][cnt1++];
                        togtmp1 = code_format[csumcodeformat - outcnt1][cnt1++];
                        togtmp2 = code_format[csumcodeformat - outcnt1][cnt1++];
                        cmpa = togtmp6 & 0x7f;
                        cmpa *= 0x100;
                        cmpa += togtmp5;
                        cmpb = togtmp4 & 0x7f;
                        cmpb *= 0x100;
                        cmpb += togtmp3;
                        cmpc = togtmp2 & 0x7f;
                        cmpc *= 0x100;
                        cmpc += togtmp1;
                        if (0 == IRDA_acmpbandc(cmpa, cmpb, cmpc)) {
                            togtmp6 = 0;
                            togtmp5 = wavedat[cntx++];
                            if (togtmp5 >= 0x80) {
                                togtmp6 = togtmp5;
                                togtmp5 = wavedat[cntx++];
                            }
                            cnt1 = 4;
                            togtmp3 = code_format[csumcodeformat - outcnt1][cnt1++];
                            togtmp4 = code_format[csumcodeformat - outcnt1][cnt1++];
                            togtmp1 = code_format[csumcodeformat - outcnt1][cnt1++];
                            togtmp2 = code_format[csumcodeformat - outcnt1][cnt1++];
                            cmpa = togtmp6 & 0x7f;
                            cmpa *= 0x100;
                            cmpa += togtmp5;
                            cmpb = togtmp4 & 0x7f;
                            cmpb *= 0x100;
                            cmpb += togtmp3;
                            cmpc = togtmp2 & 0x7f;
                            cmpc *= 0x100;
                            cmpc += togtmp1;
                            if (0 == IRDA_acmpbandc(cmpa, cmpb, cmpc)) {
                                //ToggleBit_Judge_OneBIT
                                togtmp15--;
                                if (0 == togtmp15) {    //ToggleBit_Judge_One_Format
                                    cmpbcf = 1;
                                    break;
                                } else {
                                    togtmp16 = cntx;
                                    togtmp6 = 0;
                                    togtmp5 = wavedat[cntx++];
                                    if (togtmp5 >= 0x80) {
                                        togtmp6 = togtmp5;
                                        togtmp5 = wavedat[cntx++];
                                    }
                                }
                            } else {        //ToggleBit_Judge_OneBITHigh
                                cntx = togtmp16;
                                togtmp6 = 0;
                                togtmp5 = wavedat[cntx++];
                                if (togtmp5 >= 0x80) {
                                    togtmp6 = togtmp5;
                                    togtmp5 = wavedat[cntx++];
                                }
                                cnt1 = 8;
                                togtmp3 = code_format[csumcodeformat - outcnt1][cnt1++];
                                togtmp4 = code_format[csumcodeformat - outcnt1][cnt1++];
                                togtmp1 = code_format[csumcodeformat - outcnt1][cnt1++];
                                togtmp2 = code_format[csumcodeformat - outcnt1][cnt1++];
                                cmpa = togtmp6 & 0x7f;
                                cmpa *= 0x100;
                                cmpa += togtmp5;
                                cmpb = togtmp4 & 0x7f;
                                cmpb *= 0x100;
                                cmpb += togtmp3;
                                cmpc = togtmp2 & 0x7f;
                                cmpc *= 0x100;
                                cmpc += togtmp1;
                                if (0 == IRDA_acmpbandc(cmpa, cmpb, cmpc)) {    //ToggleBit_Judge_OneBITHigh1
                                    togtmp6 = 0;
                                    togtmp5 = wavedat[cntx++];
                                    if (togtmp5 >= 0x80) {
                                        togtmp6 = togtmp5;
                                        togtmp5 = wavedat[cntx++];
                                    }
                                    cnt1 = 12;
                                    togtmp3 = code_format[csumcodeformat - outcnt1][cnt1++];
                                    togtmp4 = code_format[csumcodeformat - outcnt1][cnt1++];
                                    togtmp1 = code_format[csumcodeformat - outcnt1][cnt1++];
                                    togtmp2 = code_format[csumcodeformat - outcnt1][cnt1++];
                                    cmpa = togtmp6 & 0x7f;
                                    cmpa *= 0x100;
                                    cmpa += togtmp5;
                                    cmpb = togtmp4 & 0x7f;
                                    cmpb *= 0x100;
                                    cmpb += togtmp3;
                                    cmpc = togtmp2 & 0x7f;
                                    cmpc *= 0x100;
                                    cmpc += togtmp1;
                                    if (0 == IRDA_acmpbandc(cmpa, cmpb, cmpc)) {
                                        //ToggleBit_Judge_OneBIT
                                        togtmp15--;
                                        if (0 == togtmp15) {    //ToggleBit_Judge_One_Format
                                            cmpbcf = 1;
                                            break;
                                        } else {
                                            togtmp16 = cntx;
                                            togtmp6 = 0;
                                            togtmp5 = wavedat[cntx++];
                                            if (togtmp5 >= 0x80) {
                                                togtmp6 = togtmp5;
                                                togtmp5 = wavedat[cntx++];
                                            }
                                        }
                                    } else  //ToggleBit_Judge_Next_Format
                                        break;
                                } else      //ToggleBit_Judge_Next_Format
                                    break;
                            }
                        } else {        //ToggleBit_Judge_OneBITHigh
                            cntx = togtmp16;
                            togtmp6 = 0;
                            togtmp5 = wavedat[cntx++];
                            if (togtmp5 >= 0x80) {
                                togtmp6 = togtmp5;
                                togtmp5 = wavedat[cntx++];
                            }
                            cnt1 = 8;
                            togtmp3 = code_format[csumcodeformat - outcnt1][cnt1++];
                            togtmp4 = code_format[csumcodeformat - outcnt1][cnt1++];
                            togtmp1 = code_format[csumcodeformat - outcnt1][cnt1++];
                            togtmp2 = code_format[csumcodeformat - outcnt1][cnt1++];
                            cmpa = togtmp6 & 0x7f;
                            cmpa *= 0x100;
                            cmpa += togtmp5;
                            cmpb = togtmp4 & 0x7f;
                            cmpb *= 0x100;
                            cmpb += togtmp3;
                            cmpc = togtmp2 & 0x7f;
                            cmpc *= 0x100;
                            cmpc += togtmp1;
                            if (0 == IRDA_acmpbandc(cmpa, cmpb, cmpc)) {    //ToggleBit_Judge_OneBITHigh1
                                togtmp6 = 0;
                                togtmp5 = wavedat[cntx++];
                                if (togtmp5 >= 0x80) {
                                    togtmp6 = togtmp5;
                                    togtmp5 = wavedat[cntx++];
                                }
                                cnt1 = 12;
                                togtmp3 = code_format[csumcodeformat - outcnt1][cnt1++];
                                togtmp4 = code_format[csumcodeformat - outcnt1][cnt1++];
                                togtmp1 = code_format[csumcodeformat - outcnt1][cnt1++];
                                togtmp2 = code_format[csumcodeformat - outcnt1][cnt1++];
                                cmpa = togtmp6 & 0x7f;
                                cmpa *= 0x100;
                                cmpa += togtmp5;
                                cmpb = togtmp4 & 0x7f;
                                cmpb *= 0x100;
                                cmpb += togtmp3;
                                cmpc = togtmp2 & 0x7f;
                                cmpc *= 0x100;
                                cmpc += togtmp1;
                                if (0 == IRDA_acmpbandc(cmpa, cmpb, cmpc)) {
                                    //ToggleBit_Judge_OneBIT
                                    togtmp15--;
                                    if (0 == togtmp15) {    //ToggleBit_Judge_One_Format
                                        cmpbcf = 1;
                                        break;
                                    } else {
                                        togtmp16 = cntx;
                                        togtmp6 = 0;
                                        togtmp5 = wavedat[cntx++];
                                        if (togtmp5 >= 0x80) {
                                            togtmp6 = togtmp5;
                                            togtmp5 = wavedat[cntx++];
                                        }
                                    }
                                } else  //ToggleBit_Judge_Next_Format
                                    break;
                            } else      //ToggleBit_Judge_Next_Format
                                break;
                        }
                    }
                    if (0 == cmpbcf)
                        break;
                } else  if (commond0f2h == code_format[csumcodeformat - outcnt1][togtmp14]) {
                    return  outcnt1;
                } else
                    break;
            }
        } else  if (1 == code_format[csumcodeformat - outcnt1][16]) {
            togtmp14 = 18;  //TOGTMP14
            cntx = 5;       //---x
            togtmp16 = code_format[csumcodeformat - outcnt1][17];   //---TOGTMP16
            r_flag13 = 0;
            while (1) { //ToggleBit_Judge_One_3010
                if (0 == r_flag13) {
                    togtmp6 = 0;
                    togtmp5 = wavedat[cntx++];
                    if (togtmp5 >= 0x80) {
                        togtmp6 = togtmp5;
                        togtmp5 = wavedat[cntx++];
                    }
                }
                if (commond0f0h == code_format[csumcodeformat - outcnt1][togtmp14]) {
                    togtmp3 = code_format[csumcodeformat - outcnt1][togtmp14 + 1];
                    togtmp4 = code_format[csumcodeformat - outcnt1][togtmp14 + 2];
                    togtmp1 = code_format[csumcodeformat - outcnt1][togtmp14 + 3];
                    togtmp2 = code_format[csumcodeformat - outcnt1][togtmp14 + 4];
                    togtmp14 += 5;
                    cmpa = togtmp6 & 0x7f;
                    cmpa *= 0x100;
                    cmpa += togtmp5;
                    cmpb = togtmp4 & 0x7f;
                    cmpb *= 0x100;
                    cmpb += togtmp3;
                    cmpc = togtmp2 & 0x7f;
                    cmpc *= 0x100;
                    cmpc += togtmp1;
                    if (0 == IRDA_acmpbandc(cmpa, cmpb, cmpc)) {
                        //ToggleBit_Judge_One_30102
                        r_flag13 = 0;
                        continue;
                    } else  if (0x0f == IRDA_acmpbandc(cmpa, cmpb, cmpc) || r_flag13) {
                        //noendhe3010
                        //ToggleBit_Judge_Next_Format
                        break;
                    } else {
                        cmpb += cmpc;
                        cmpb /= 2;
                        cmpa -= cmpb;
                        togtmp6 = cmpa / 0x100;
                        togtmp5 = cmpa % 0x100;
                        if (togtmp6 > 0 || togtmp5 > 0x7f)
                            togtmp6 |= 0x80;
                        r_flag13 = 1;
                        continue;
                    }
                } else  if (commond0f1h == code_format[csumcodeformat - outcnt1][togtmp14]) {
                    //nopuslehe3010
                    //nodatahe3010
                    togtmp15 = code_format[csumcodeformat - outcnt1][togtmp14 + 1]; //togtmp15
                    togtmp15 *= 2;
                    if ((togtmp16 & 0x1) == 1)
                        togtmp15--;
                    togtmp14 += 2;
                    cnt1 = 0;
                    togtmp3 = code_format[csumcodeformat - outcnt1][cnt1++];
                    togtmp4 = code_format[csumcodeformat - outcnt1][cnt1++];
                    togtmp1 = code_format[csumcodeformat - outcnt1][cnt1++];
                    togtmp2 = code_format[csumcodeformat - outcnt1][cnt1++];
                    progf1 = 0;
                    while (1) { //ToggleBit_Judge_3010BIT0
                        cmpa = togtmp6 & 0x7f;
                        cmpa *= 0x100;
                        cmpa += togtmp5;
                        cmpb = togtmp4 & 0x7f;
                        cmpb *= 0x100;
                        cmpb += togtmp3;
                        cmpc = togtmp2 & 0x7f;
                        cmpc *= 0x100;
                        cmpc += togtmp1;
                        if (0 == IRDA_acmpbandc(cmpa, cmpb, cmpc)) {
                            //ToggleBit_Judge_3010OneBIT
                            r_flag13 = 0;
                            //ToggleBit_Judge_3010OneBIT3
                            togtmp15--;
                            if (0 == togtmp15)
                                break;
                            else {
                                if (0 == r_flag13) {
                                    togtmp6 = 0;
                                    togtmp5 = wavedat[cntx++];
                                    if (togtmp5 >= 0x80) {
                                        togtmp6 = togtmp5;
                                        togtmp5 = wavedat[cntx++];
                                    }
                                }
                            }
                            //goto  ToggleBit_Judge_3010BIT0
                        } else  if (0x0f == IRDA_acmpbandc(cmpa, cmpb, cmpc) || r_flag13) {
                            //noendhe3010
                            //ToggleBit_Judge_Next_Format
                            progf1 = 1;
                            break;
                        } else {
                            cmpb = code_format[csumcodeformat - outcnt1][15] & 0x7f;
                            cmpb *= 0x100;
                            cmpb += code_format[csumcodeformat - outcnt1][14];
                            cmpa -= cmpb;
                            togtmp6 = cmpa / 0x100;
                            togtmp5 = cmpa % 0x100;
//                          if(togtmp6>0||togtmp5>0x7f)
//                              togtmp6|=0x80;
                            r_flag13 = 1;
                            //ToggleBit_Judge_3010OneBIT3
                            togtmp15--;
                            if (0 == togtmp15)
                                break;
                            else {
                                if (0 == r_flag13) {
                                    togtmp6 = 0;
                                    togtmp5 = wavedat[cntx++];
                                    if (togtmp5 >= 0x80) {
                                        togtmp6 = togtmp5;
                                        togtmp5 = wavedat[cntx++];
                                    }
                                }
                            }
                            //goto  ToggleBit_Judge_3010BIT0
                        }
                    }
                    if (progf1)
                        break;
                } else  if (commond0f2h == code_format[csumcodeformat - outcnt1][togtmp14]) {
                    return  outcnt1;
                } else
                    //ToggleBit_Judge_Next_Format
                    break;
            }
        } else {
            return  0;
        }
    }
    return  0;
}

//====================
void    IRDA_Reload_Data_Buff(czx_vu32 inhb, czx_vu32 inlb, czx_vu32 *wavedat)
{
    if (inhb == 0) {
        if (inlb >= 0x80)
            wavedat[cntx++] = 0x80;
        wavedat[cntx++] = inlb;
    } else {
        wavedat[cntx++] = inhb | 0x80;
        wavedat[cntx++] = inlb;
    }
    return;
}

czx_u32 IRDA_keytogglebit(czx_vu32 *wavedat, czx_vu32 *indatsum)        ///wjs;IN:sendlrndat[] 110B  Y
{
    czx_vu32    tmp1;
    czx_vu32    tmp2;

    if ((wavedat[1] && 0x07) == 0)
        return  0;

    else {
        wavedat[1] ^= 0x10;
        tmp1 = IRDA_JudgeToggleBit(wavedat, indatsum);      //wjs;Y
        if (0 != tmp1) {
//          kal_prompt_trace(MOD_MMI, "liuyingbin_remote_togglebit = %d",tmp1);
            IRDA_changetogglebit(wavedat, indatsum, tmp1);  //wjs;Y
        }
        tmp2 = wavedat[1];
        tmp2 &= 0xf0;
        tmp2 |= tmp1;
    }
    return  tmp2;
}
void    IRDA_changetogglebit(czx_vu32 *wavedat, czx_vu32    *indatsum, czx_vu32 togtmp9)
{
    czx_vu32    togtmp6, togtmp5, togtmp4, togtmp3, togtmp2, togtmp1;
    czx_vu32    togtmp14, togtmp15, togtmp16, togtmp17, togtmp7, togtmp8, togtmp12;
    czx_vu32    cmpa, cmpb, cmpc;
    czx_vu32    cnt1, tmpd1, tmpd2;
    czx_vu32    togt9;
//  unsigned    int cnty,cntx;
    czx_vu8     r_flag13;
    czx_vu8     r_flag17;
    czx_vu8     r_flag10;
    czx_vu8     r_flag11;

    if (1 == togtmp9 || 0 == togtmp9)
        return;
    else    if (togtmp9 >= 5) {
        togtmp9 = 8 - togtmp9;
        togtmp1 = ToggleBit_Place[togtmp9];     //wjs;y
        cntx = 5;
        for (cnt1 = togtmp1; cnt1 != 0; cnt1--) {
            togtmp14 = cntx;
            togtmp6 = 0;
            togtmp5 = wavedat[cntx++];
            if (togtmp5 >= 0x80) {
                togtmp6 = togtmp5;
                togtmp5 = wavedat[cntx++];
            }
        }
        cnty = 0;
        while (1) {
            togtmp4 = 0;
            togtmp3 = wavedat[cntx++];
            if (togtmp3 >= 0x80) {
                togtmp4 = togtmp3;
                togtmp3 = wavedat[cntx++];
            }
            togtmp4 = 0;
            togtmp3 = wavedat[cntx++];
            if (togtmp3 >= 0x80) {
                togtmp4 = togtmp3;
                togtmp3 = wavedat[cntx++];
            }
            togtmp7 = togtmp3;
            togtmp8 = togtmp4;
            tmpd1 = togtmp6 & 0x7f;
            tmpd1 *= 0x100;
            tmpd1 += togtmp5;
            tmpd2 = togtmp4 & 0x7f; //TOGTMP13
            tmpd2 *= 0x100;
            tmpd2 += togtmp3;
            cnt1 = wavedat[1] & 0x80;
            if (0 != IRDA_cmpaequbtog(tmpd1, tmpd2, cnt1))
                break;
        }
        if (tmpd2 < tmpd1) {
xiaoyu1:
            if (0 != (wavedat[1] & 0x10))
                return;
            else {
                //xiaoyudayu4:
                goto    xiaoyudayu4;
            }
        } else {
            //xiaoyudayu1:
            tmpd2 -= tmpd1;
            cntx = togtmp9 * 2;
            cntx *= 2;
            cntx++;
            togtmp4 = STandantdata[cntx--];
            togtmp3 = STandantdata[cntx--];
            tmpd1 = togtmp4 & 0x7f;
            tmpd1 *= 0x100;
            tmpd1 += togtmp3;
            if (tmpd1 >= tmpd2) {
xiaoyudayu5:
                if (0 == (wavedat[1] & 0x10))
                    return;
                else {
xiaoyudayu4:
                    cntx = togtmp14;
                    togtmp15 = wavedat[cntx];
                    if (togtmp7 >= 0x80 || togtmp8 != 0) {
                        //PrecossToggleBit_3004_b4:
                        if (togtmp15 < 0x80) {
                            IRDA_Some_Data_Right_Move1(wavedat, indatsum, togtmp14);
                        }
                        cntx = togtmp14;
                        wavedat[cntx++] = togtmp8 | 0x80;
                        wavedat[cntx++] = togtmp7;
                        return;
                    } else {
                        //PrecossToggleBit_3004_b3:
                        if (togtmp15 >= 0x80)
                            wavedat[cntx++] = 0;
                        wavedat[cntx++] = togtmp7;
                        return;
                    }
                }
            } else {
                //getstandatatogl:
                cntx = togtmp9 * 2;
                cntx *= 2;
                togtmp7 = STandantdata[cntx++];
                togtmp3 = togtmp7;
                togtmp8 = STandantdata[cntx];
                togtmp4 = togtmp8;
                tmpd1 = togtmp6 & 0x7f;
                tmpd1 *= 0x100;
                tmpd1 += togtmp5;
                tmpd2 = togtmp4 & 0x7f;
                tmpd2 *= 0x100;
                tmpd2 += togtmp3;
                cnt1 = wavedat[1] & 0x80;
                if (0 == IRDA_cmpaequbtog(tmpd1, tmpd2, cnt1)) {
                    cntx = togtmp9 * 2;
                    cntx *= 2;
                    cntx += 2;
                    togtmp7 = STandantdata[cntx++];
                    togtmp3 = togtmp7;
                    togtmp8 = STandantdata[cntx];
                    togtmp4 = togtmp8;
                    goto    xiaoyu1;
                } else
                    goto    xiaoyudayu5;
            }
        }
    } else {
        togt9 = togtmp9;
        togtmp9 = ToggleBit_Place[8 - togtmp9];
        togtmp14 = 18;
        cntx = 5;
        togtmp16 = code_format[csumcodeformat - togt9][17];
        r_flag13 = 0;
        r_flag17 = 1;
        r_flag10 = 1;
PrecossToggleBit_One_3010: {
            if (0 == r_flag13) {
                togtmp17 = cntx;
                togtmp6 = 0;
                togtmp5 = wavedat[cntx++];
                if (togtmp5 >= 0x80) {
                    togtmp6 = togtmp5;
                    togtmp5 = wavedat[cntx++];
                }
            }
            if (commond0f0h == code_format[csumcodeformat - togt9][togtmp14]) {
                togtmp3 = code_format[csumcodeformat - togt9][togtmp14 + 1];
                togtmp4 = code_format[csumcodeformat - togt9][togtmp14 + 2];
                togtmp1 = code_format[csumcodeformat - togt9][togtmp14 + 3];
                togtmp2 = code_format[csumcodeformat - togt9][togtmp14 + 4];
                togtmp14 += 5;
                cmpa = togtmp6 & 0x7f;
                cmpa *= 0x100;
                cmpa += togtmp5;
                cmpb = togtmp4 & 0x7f;
                cmpb *= 0x100;
                cmpb += togtmp3;
                cmpc = togtmp2 & 0x7f;
                cmpc *= 0x100;
                cmpc += togtmp1;
                if (0 == IRDA_acmpbandc(cmpa, cmpb, cmpc)) {
                    //PrecossToggleBit_One_30102:
                    r_flag17 ^= 1;
                    r_flag13 = 0;
                    goto    PrecossToggleBit_One_3010;
                } else  if (0x0f == IRDA_acmpbandc(cmpa, cmpb, cmpc) || r_flag13 == 1) {
                    return;
                } else {
                    tmpd1 = cmpc + cmpb;
                    tmpd1 /= 2;
                    cmpa -= tmpd1;
                    togtmp6 = cmpa / 0x100;
                    togtmp5 = cmpa % 0x100;
                    if (togtmp6 > 0 || togtmp5 > 0x7f)
                        togtmp6 |= 0x80;
                    r_flag13 = 1;
                    goto    PrecossToggleBit_One_3010;
                }
            } else  if (commond0f1h == code_format[csumcodeformat - togt9][togtmp14]) {
                //PrecossToggleBit_nodatahe3010:
                togtmp15 = code_format[csumcodeformat - togt9][togtmp14 + 1];
                togtmp14 += 2;
                togtmp3 = code_format[csumcodeformat - togt9][0];
                togtmp4 = code_format[csumcodeformat - togt9][1];
                togtmp1 = code_format[csumcodeformat - togt9][2];
                togtmp2 = code_format[csumcodeformat - togt9][3];
                r_flag11 = 1;
PrecossToggleBit_3010BIT0:
                cmpa = togtmp6 & 0x7f;
                cmpa *= 0x100;
                cmpa += togtmp5;
                cmpb = togtmp4 & 0x7f;
                cmpb *= 0x100;
                cmpb += togtmp3;
                cmpc = togtmp2 & 0x7f;
                cmpc *= 0x100;
                cmpc += togtmp1;
                if (0 == IRDA_acmpbandc(cmpa, cmpb, cmpc)) {
                    //PrecossToggleBit_3010OneBIT:
                    r_flag17 ^= 1;
                    r_flag13 = 0;
                    goto    PrecossToggleBit_3010OneBIT3;
                } else {
                    cmpb = code_format[csumcodeformat - togt9][15] & 0x7f;
                    cmpb *= 0x100;
                    cmpb += code_format[csumcodeformat - togt9][14];
                    cmpa -= cmpb;
                    togtmp6 = cmpa / 0x100;
                    togtmp5 = cmpa % 0x100;
                    if (togtmp6 > 0 || togtmp5 > 0x7f)
                        togtmp6 |= 0x80;
                    r_flag13 = 1;
PrecossToggleBit_3010OneBIT3:
                    if (1 != togtmp16 || r_flag10 == 0) {
                        //PrecossToggleBit_nolost1bit:
                        r_flag11 ^= 1;
                        if (0 == r_flag11)
                            goto    PrecossToggleBit_3010OneBIT2;
                        else
                            goto    PrecossToggleBit_3010OneBIT4;
                    } else {
                        r_flag10 = 0;
PrecossToggleBit_3010OneBIT4:
                        r_flag11 = 1;
                        togtmp9--;
                        if (0 == togtmp9) {
                            //Get_ToggleBit_Place_3010:
                            //if((r_flag17==1&&0!=(wavedat[1]&0x10))||(r_flag17==0&&0==(wavedat[1]&0x10)))
                            if ((r_flag17 == 1 && 0 == (wavedat[1] & 0x10)) || (r_flag17 == 0 && 0 != (wavedat[1] & 0x10)))
                                return;
                            else {
                                //thisbitis3:
                                r_flag10 = 1;
                                tmpd2 = 0;
                                togtmp17 = IRDA_All_Send_Data_Right_Move(wavedat, indatsum, togtmp17);
                                togtmp3 = code_format[csumcodeformat - togt9][10];
                                togtmp4 = code_format[csumcodeformat - togt9][11];
                                togtmp6 = 0;
                                togtmp5 = wavedat[cnty++];
                                if (togtmp5 >= 0x80) {
                                    togtmp6 = togtmp5;
                                    togtmp5 = wavedat[cnty++];
                                }
                                togtmp1 = code_format[csumcodeformat - togt9][12];
                                togtmp2 = code_format[csumcodeformat - togt9][13];
                                cmpa = togtmp6 & 0x7f;
                                cmpa *= 0x100;
                                cmpa += togtmp5;
                                cmpb = togtmp2 & 0x7f;
                                cmpb *= 0x100;
                                cmpb += togtmp1;
                                if (cmpa >= cmpb)   //ACMPB
                                    r_flag13 = 1;
                                else
                                    r_flag13 = 0;
Get_ToggleBit_Place_3010Start:
                                if (0 == r_flag13) {
                                    cmpb = togtmp4 & 0x7f;
                                    cmpb *= 0x100;
                                    cmpb += togtmp3;
                                    cmpa += cmpb;
                                    togtmp1 = code_format[csumcodeformat - togt9][8];
                                    togtmp2 = code_format[csumcodeformat - togt9][9];
                                    cmpb = togtmp2 & 0x7f;
                                    cmpb *= 0x100;
                                    cmpb += togtmp1;
                                    togtmp6 = cmpa / 0x100;
                                    togtmp5 = cmpa % 0x100;
                                    if (togtmp6 > 0 || togtmp5 > 0x7f)
                                        togtmp6 |= 0x80;
                                    togtmp12 = cmpa % 0x100;
                                    if (cmpa >= cmpb)   //ACMPB2
                                        r_flag13 = 1;
                                    else
                                        r_flag13 = 0;
                                    if (r_flag13) {
                                        //ASUBB:
                                        togtmp1 = code_format[csumcodeformat - togt9][14];
                                        togtmp2 = code_format[csumcodeformat - togt9][15];
                                        cmpb = togtmp2 & 0x7f;
                                        cmpb *= 0x100;
                                        cmpb += togtmp1;
                                        cmpa -= cmpb;
                                        togtmp6 = cmpa / 0x100;
                                        togtmp5 = cmpa % 0x100;
                                        if (togtmp6 > 0 || togtmp5 > 0x7f)
                                            togtmp6 |= 0x80;
                                        togtmp12 = cmpa % 0x100;
                                    }
                                    //PToggleBit_3010nsbc1:
                                    IRDA_Reload_Data_Buff(togtmp6, togtmp12, wavedat);
                                    togtmp6 = 0;
                                    togtmp5 = wavedat[cnty++];
                                    if (togtmp5 >= 0x80) {
                                        togtmp6 = togtmp5;
                                        togtmp5 = wavedat[cnty++];
                                    }
                                    wavedat[2]--;
                                    wavedat[3]--;
                                    goto    PToggleBit_3010_next_pulse;
                                } else {
                                    //PToggleBit_3010sbc:
                                    wavedat[2]++;
                                    wavedat[3]++;
                                    r_flag10 ^= 1;
                                    if (r_flag10 == 1) {
                                        togtmp12 = togtmp3;
                                        IRDA_Reload_Data_Buff(togtmp4, togtmp12, wavedat);
                                        cmpa = togtmp6 & 0x7f;
                                        cmpa *= 0x100;
                                        cmpa += togtmp5;
                                        cmpb = togtmp4 & 0x7f;
                                        cmpb *= 0x100;
                                        cmpb += togtmp3;
                                        cmpa -= cmpb;
                                        togtmp6 = cmpa / 0x100;
                                        togtmp5 = cmpa % 0x100;
                                        if (togtmp6 > 0 || togtmp5 > 0x7f)
                                            togtmp6 |= 0x80;
                                        togtmp12 = cmpa % 0x100;
                                        IRDA_Reload_Data_Buff(togtmp6, togtmp12, wavedat);
                                        goto    PToggleBit_3010_next_pulse;
                                    } else {
                                        //subfirwave1:
                                        cmpa = togtmp6 & 0x7f;
                                        cmpa *= 0x100;
                                        cmpa += togtmp5;
                                        cmpb = togtmp4 & 0x7f;
                                        cmpb *= 0x100;
                                        cmpb += togtmp3;
                                        cmpa -= cmpb;
                                        togtmp12 = cmpa % 0x100;
                                        IRDA_Reload_Data_Buff(cmpa / 0x100, togtmp12, wavedat);
                                        togtmp12 = togtmp3;
                                        IRDA_Reload_Data_Buff(togtmp4, togtmp12, wavedat);
PToggleBit_3010_next_pulse:
                                        tmpd2++;
                                        if (tmpd2 >= 2) {
                                            //All_Send_Data_Left_Move:
                                            cnt1 = *indatsum - 4;
                                            if (cnty < cnt1) {
                                                while (cnty < cnt1) {
                                                    wavedat[cntx++] = wavedat[cnty++];
                                                }
                                            }
                                            for (cnt1 = 0; cnt1 < 4; cnt1++) {
                                                wavedat[cntx++] = wavedat[cnty++];
                                            }
                                        } else {
                                            togtmp6 = 0;
                                            togtmp5 = wavedat[cnty++];
                                            if (togtmp5 >= 0x80) {
                                                togtmp6 = togtmp5;
                                                togtmp5 = wavedat[cnty++];
                                            }
                                            togtmp1 = code_format[csumcodeformat - togt9][12];
                                            togtmp2 = code_format[csumcodeformat - togt9][13];
                                            cmpa = togtmp6 & 0x7f;
                                            cmpa *= 0x100;
                                            cmpa += togtmp5;
                                            cmpb = togtmp2 & 0x7f;
                                            cmpb *= 0x100;
                                            cmpb += togtmp1;
                                            if (cmpa >= cmpb)   //ACMPB
                                                r_flag13 = 1;
                                            else
                                                r_flag13 = 0;
                                            goto    Get_ToggleBit_Place_3010Start;
                                        }
                                    }
                                }
                            }
                        } else {
                            togtmp15--;
                            if (0 == togtmp15)
                                goto    PrecossToggleBit_One_3010;
                            else {
PrecossToggleBit_3010OneBIT2:
                                if (0 == r_flag13) {
                                    togtmp17 = cntx;
                                    togtmp6 = 0;
                                    togtmp5 = wavedat[cntx++];
                                    if (togtmp5 >= 0x80) {
                                        togtmp6 = togtmp5;
                                        togtmp5 = wavedat[cntx++];
                                    }
                                    goto    PrecossToggleBit_3010BIT0;
                                } else
                                    goto    PrecossToggleBit_3010BIT0;
                            }
                        }
                    }
                }
            } else {
                return;
            }
        }
    }
    return;
}
czx_vu8 g_toggle_bit = 0;
czx_vu32 sendlrndat[109 + 1];
czx_u8 IRDA_send_remote_study_data(czx_vu8 *cmd_data, czx_vu16 cmd_lnth)        //wjs;cmd_lnth=109
{
    czx_vu32  sendlrndatsum;
    //char g_i2c_cmd_buffer[109+3] = "";
    czx_vu32 toggle;
    czx_vu8 temp;
    czx_vu8 checksum = 0x32;    ///40;  //wjs;YL:START_WRITE_I2C_DADDR + SEND_STUDY_REMOTE_DATA;


    sendlrndatsum = cmd_lnth;


    for (temp = 0; temp < cmd_lnth; temp++)         ///wjs;cmd_lnth ＝109
        sendlrndat[temp + 1] = cmd_data[temp];      //wjs;cmd_data[]：110Ｂ处理后，这数据只有这里有用；；sendlrndat[]：110Ｂ

    sendlrndat[0] = 0x00;
    sendlrndatsum = 110;

    g_toggle_bit++;
    if (g_toggle_bit &= 0x01)
        sendlrndat[1] ^= 0x10;

    toggle = IRDA_keytogglebit(sendlrndat, &sendlrndatsum); //wjs;包括了各种数据处理

    g_i2c_cmd_buffer[0] = 0X30;     ///wjs;YL:START_WRITE_I2C_DADDR;    //wjs;0X30
    g_i2c_cmd_buffer[1] = 0x02;     ////wjs;YL: SEND_STUDY_REMOTE_DATA; //wjs;0X02

    for (temp = 0; temp < cmd_lnth; temp++) {
        g_i2c_cmd_buffer[2 + temp] = sendlrndat[temp + 1];
        checksum += g_i2c_cmd_buffer[2 + temp];
    }

    g_i2c_cmd_buffer[111] = checksum;
    return cmd_lnth;
//  templnth = 112;             ;;wjs;2012-7-10 21:36;原来是通过ＩＯ通讯发出的部分
//
//  I2CSetTransferMode(0);
//  writeI2C(g_i2c_cmd_buffer, templnth);
}
czx_u8 IRDA_compdata(czx_vu32   *wavedat, czx_vu32  *indatsum)
{
    czx_vu32    *intp1;
    czx_vu32    tmp1, tmp2;
    czx_vu8 flag1 = 0;

    intp1 = indatsum;
    (*intp1)--;

    tmp1 = wavedat[0];
    if (tmp1 != 0) {
        //kal_prompt_trace(MOD_MMI, "remote_study_data == Study_type_error");
        return 1;
    }

    else {
        tmp1 = wavedat[1];
        if (tmp1 >= 0x80) {
            flag1 = IRDA_Modifywave(wavedat, intp1);
            IRDA_delfeng(wavedat, intp1);
        } else {
            flag1 = IRDA_modifywavem708(wavedat, intp1);
        }

        if (flag1 == 0) {
            flag1 = IRDA_getfigure(wavedat, intp1);
        } else {
            //kal_prompt_trace(MOD_MMI, "remote_study_data == Study_wave_error");
            return 1;
        }
    }

    return 0;
}
czx_vu8 readtempbuf[110];
czx_vu32 irprocessbuf[111];
czx_u8      IRDA_get_remote_study_data(czx_vu8 *cmd_data)
{
    czx_vu8 temp;
    czx_vu32 datalength;

    temp = IRDA_rx_data(readtempbuf);       //发学习命令前，要清readtempbuf

    if (!temp)
        return 0;

    else {
        for (temp = 0; temp < 109 + 1; temp++) {
            irprocessbuf[temp] = (czx_vu32)readtempbuf[temp];       ///wjs;readtempbuf,irprocessbuf原始110Ｂ:未处理
        }
        datalength = 109 + 1;
        if (0 != (IRDA_compdata(irprocessbuf, &datalength))) {
            return 0;
        } else {
            for (temp = 0; temp < 109; temp++) {    //wjs;datalengt
                cmd_data[temp] = (czx_vu8 )irprocessbuf[temp + 1];      //wjs;cmd_data[]:110B处理过程中的数据:109B
            }
        }

        return datalength;
    }

}



czx_vu8  g_remote_study_data_buf[110];
czx_u8      IRDA_learn_data_in_out(czx_vu8  * learn_data_out)
{
    czx_vu8     tempw;
    czx_vu8         tempct;         //增加这个变量
    czx_vu8 data_check = 0;         //增加这个变量
    czx_vu32 i = 0;
    //unsigned char  g_remote_study_data_buf[112];

    ///////=================================================================//wjs;为解决得到学习错误数据问题
    tempct = IRDA_get_remote_study_data((czx_vu8 *)learn_data_out);
    if (!tempct) {
        for ( i = 0; i < 112; i++) {
            learn_data_out[i] = 0;
        }
        return 0;
    } else {
        for ( i = 0; i < 109; i++) {
            g_remote_study_data_buf[i] = learn_data_out[i];
        }
        IRDA_send_remote_study_data((czx_vu8 *)g_remote_study_data_buf, 109);   //wjs;;
        for (tempw = 0; tempw < 112; tempw++) {
            learn_data_out[tempw] = g_i2c_cmd_buffer[tempw];        //wjs;得到的学习数据
        }
        //*********************************************增加：错误数据，数据清0
        for (tempw = 10; tempw < 15; tempw++) {
            data_check  += (learn_data_out[tempw]);
        }
        if (learn_data_out[3] < 5 || learn_data_out[5] < 0x10 || learn_data_out[5] > 0x80) {    //原来：if(!data_check)
            for (int i = 0; i < 112; i++) {
                learn_data_out[i] = 0;
            }
            return 0;
        }
        //***************************
        return 112;
    }
}
czx_u8      IRDA_Modifywave(czx_vu32*wavedat, czx_vu32*indatsum)
{
    czx_vu32 lrn_flag, receive_time, repdatalen;
    czx_vu32    waveld1 = 0, waveld2 = 0;
    czx_vu32    wavedd1, wavedd2;
    czx_vu32    cnt1, cnt2, cnt3, cnt4;

    if (wavedat[0] == 0) {
        lrn_flag = wavedat[1];          //wjs;标志变量
        receive_time = wavedat[2];
        repdatalen = wavedat[3];
        cnt3 = 0;       //parity--//receive_time
        cnt1 = 5;       //point-y===cnt2---point-x
        for (cnt2 = 5; cnt2 < *indatsum + 1; cnt2++) {

            wavedd1 = wavedat[cnt2];
            if (wavedd1 >= 0x80) {
                wavedd2 = wavedd1 & 0x7f;
                cnt2++;
                if (cnt2 == *indatsum + 1) {
                    wavedd1 = 0;
                } else {
                    wavedd1 = wavedat[cnt2];
                }
            } else {
                wavedd2 = 0;
            }
            waveld1 = wavedd2 * 0x100;
            waveld1 += wavedd1;
            if ((cnt3 & 0x01) == 0) {
                if (waveld1 > 7) {
                    waveld1 -= 7;
                } else
                    waveld1 = 2;
            } else {
                waveld1 += 3;
            }
            wavedd2 = waveld1 / 0x100;
            wavedd1 = waveld1 % 0x100;
            if ((wavedd2 > 0) || (wavedd1 >= 0x80)) {
                wavedd2 |= 0x80;
                wavedat[cnt1++] = wavedd2;
                wavedat[cnt1++] = wavedd1;
            } else
                wavedat[cnt1++] = wavedd1;
            cnt3++;
            if (cnt3 >= receive_time) {
                receive_time++;
                break;
            }
        }
        receive_time--;
        cnt4 = 5;
        if (wavedat[5] >= 0x80) {               //wjs;相当于9ms的数据
            cnt4 = 7;
            if (wavedat[6] < 253) {
                wavedat[6] += 3;
            } else {
                wavedat[6] += 3;
                wavedat[6] -= 0x100;
                wavedat[5]++;
            }
        } else {
            cnt4 = 6;
            if (wavedat[5] < 126)
                wavedat[5] += 2;
            else
                wavedat[5] = 127;
        }
        if (wavedat[cnt4] >= 0x80) {
            if (wavedat[cnt4 + 1] != 0) {
                wavedat[cnt4 + 1] -= 1;
            } else {
                wavedat[cnt4 + 1] = 255;
                wavedat[cnt4] &= 0x7f;
                wavedat[cnt4]--;
                wavedat[cnt4] |= 0x80;
            }
        } else {
            if (wavedat[cnt4] < 2) {
                wavedat[cnt4] = 2;
            }
        }
        return  0;
    } else
        return  1;
    //return    0;  //ok
}
