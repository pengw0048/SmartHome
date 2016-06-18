#ifndef DHT_H
#define DHT_H
#include <stdint.h>

/* DHT library
MIT license
written by Adafruit Industries
ported bt YuXiang Zhang
*/

// how many timing transitions we need to keep track of. 2 * number bits + extra
#define MAXTIMINGS 85

#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21


void DHT_Init(uint8_t type);
float DHT_ReadTemperature();
float DHT_ConvertCtoF(float);
float DHT_ConvertFtoC(float);
float DHT_ComputeHeatIndex(float tempFahrenheit, float percentHumidity);
float DHT_ReadHumidity(void);


#endif