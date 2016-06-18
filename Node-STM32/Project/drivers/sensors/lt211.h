#ifndef LT211_H__
#define LT211_H__

#include "common.h"

enum LT211_VALUE_TYPE
{
    LT211_VAL_VOLT,
    LT211_VAL_AMP,
    LT211_VAL_WATT,
    LT211_VAL_KW_H
};

bool LT211_Init(void);
bool LT211_ReadValueF(float* value, uint8_t type);

#endif