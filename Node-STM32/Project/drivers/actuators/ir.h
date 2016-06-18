#ifndef IR_H__
#define IR_H__

void IR_Init(void);
bool IR_StartLearning(uint8_t channel);
bool IR_Emit(uint8_t channel);

#endif