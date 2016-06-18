#ifndef _USB_COMMON_H__
#define _USB_COMMON_H__
#include "usb_type.h"


bool USBDevice_PlugIn();
void Set_USBClock(void);
void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
void USB_Interrupts_Config(void);
void USB_Cable_Config (FunctionalState NewState);
void Get_SerialNum(void);
void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len);

void USBCommon_Init(void);
#endif
