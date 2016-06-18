/*
    SFE_BMP180.h
    Bosch BMP180 pressure sensor library for the Arduino microcontroller
    Mike Grusin, SparkFun Electronics

    Uses floating-point equations from the Weather Station Data Logger project
    http://wmrx00.sourceforge.net/
    http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf

    Forked from BMP085 library by M.Grusin
    Ported to STM32 by YuXiang Zhang

    version 1.0 2013/09/20 initial version

    Our example code uses the "beerware" license. You can do anything
    you like with this code. No really, anything. If you find it useful,
    buy me a (root) beer someday.
*/

#ifndef SFE_BMP180_h
#define SFE_BMP180_h

char BMP180_Init(void);
// call pressure.begin() to initialize BMP180 before use
// returns 1 if success, 0 if failure (bad component or I2C bus shorted?)

char BMP180_StartTemperature(void);
// command BMP180 to start a temperature measurement
// returns (number of ms to wait) for success, 0 for fail

char BMP180_GetTemperature(double *T);
// return temperature measurement from previous startTemperature command
// places returned value in T variable (deg C)
// returns 1 for success, 0 for fail

char BMP180_StartPressure(char oversampling);
// command BMP180 to start a pressure measurement
// oversampling: 0 - 3 for oversampling value
// returns (number of ms to wait) for success, 0 for fail

char BMP180_GetPressure(double *P, double *T);
// return absolute pressure measurement from previous startPressure command
// note: requires previous temperature measurement in variable T
// places returned value in P variable (Pa)
// returns 1 for success, 0 for fail

double BMP180_Altitude(double P, double P0);
// convert absolute pressure to altitude (given baseline pressure; sea-level, runway, etc.)
// P: absolute pressure (Pa)
// P0: fixed baseline pressure (Pa)
// returns signed altitude in meters


#define BMP180_ADDR 0x77 // 7-bit address

#define BMP180_REG_CONTROL 0xF4
#define BMP180_REG_RESULT 0xF6

#define BMP180_COMMAND_TEMPERATURE 0x2E
#define BMP180_COMMAND_PRESSURE0 0x34
#define BMP180_COMMAND_PRESSURE1 0x74
#define BMP180_COMMAND_PRESSURE2 0xB4
#define BMP180_COMMAND_PRESSURE3 0xF4

#endif
