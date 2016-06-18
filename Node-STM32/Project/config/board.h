#ifndef BOARD_H__
#define BOARD_H__

//---------传感器----------------------------

//1.选择湿度传感器11111
//#define ENABLE_DHT11

//2.选择可燃气体传感器111
// #define ENABLE_MQ2

//4.选择射频卡读卡器
// #define ENABLE_PN532

//5.选择光线传感器
// #define ENABLE_BH1750

//6.选择光线传感器
//#define ENABLE_TSL2561

//7.选择气压传感器
// #define ENABLE_BMP180

//8.选择热释电传感器
// #define ENABLE_SR501

//9.选择电功率表
//#define ENABLE_LT211

//10.选择通用模拟量传感器（雨水、土壤湿度）
// #define ENABLE_ANALOG

//-------执行器-----------

//1.选择继电器
// #define ENABLE_SWITCH

//2.选择红外遥控
// #define ENABLE_IR

//3.选择PWM输出，舵机、功率控制等
 #define ENABLE_PWM

#endif
