#ifndef PWM_H__
#define PWM_H__


void PWM_Init(int freq);
void PWM_Channel_Precise(int channel, int percent_100, uint8_t bEnabled);
void PWM_Channel(int channel, int percent, uint8_t bEnabled);

#endif