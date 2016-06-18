/*
    SFE_BMP180.cpp
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

#include "bmp180.h"
#include "common.h"
#include "i2c.h"
#include <stdio.h>
#include <math.h>

static int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
static uint16_t ac4, ac5, ac6;
static uint8_t oss;

static char readInt(char address, int16_t *value);
// read an signed int (16 bits) from a BMP180 register
// address: BMP180 register address
// value: external signed int for returned value (16 bits)
// returns 1 for success, 0 for fail, with result in value

static char readUInt(char address, uint16_t *value);
// read an unsigned int (16 bits) from a BMP180 register
// address: BMP180 register address
// value: external unsigned int for returned value (16 bits)
// returns 1 for success, 0 for fail, with result in value

static char readBytes(unsigned char *values, char length);
// read a number of bytes from a BMP180 register
// values: array of char with register address in first location [0]
// length: number of bytes to read back
// returns 1 for success, 0 for fail, with read bytes in values[] array

static char writeBytes(unsigned char *values, char length);
// write a number of bytes to a BMP180 register (and consecutive subsequent registers)
// values: array of char with register address in first location [0]
// length: number of bytes to write
// returns 1 for success, 0 for fail


char BMP180_Init()
// Initialize library for subsequent pressure measurements
{
    I2C_Lib_Init();

    // The BMP180 includes factory calibration data stored on the device.
    // Each device has different numbers, these must be retrieved and
    // used in the calculations when taking pressure measurements.

    // Retrieve calibration data from device:

    if (readInt(0xAA, &ac1) &&
            readInt(0xAC, &ac2) &&
            readInt(0xAE, &ac3) &&
            readUInt(0xB0, &ac4) &&
            readUInt(0xB2, &ac5) &&
            readUInt(0xB4, &ac6) &&
            readInt(0xB6, &b1) &&
            readInt(0xB8, &b2) &&
            readInt(0xBA, &mb) &&
            readInt(0xBC, &mc) &&
            readInt(0xBE, &md)) {

        // All reads completed successfully!

        // If you need to check your math using known numbers,
        // you can uncomment one of these examples.
        // (The correct results are commented in the below functions.)

        // Example from Bosch datasheet
        // ac1 = 408; ac2 = -72; ac3 = -14383; ac4 = 32741; ac5 = 32757; ac6 = 23153;
        // B1 = 6190; B2 = 4; mb = -32768; mc = -8711; md = 2868;

        // Example from http://wmrx00.sourceforge.net/Arduino/BMP180-Calcs.pdf
        // ac1 = 7911; ac2 = -934; ac3 = -14306; ac4 = 31567; ac5 = 25671; ac6 = 18974;
        // b1 = 5498; b2 = 46; mb = -32768; mc = -11075; md = 2432;

        /*
        Serial.print("ac1: "); Serial.println(ac1);
        Serial.print("ac2: "); Serial.println(ac2);
        Serial.print("ac3: "); Serial.println(ac3);
        Serial.print("ac4: "); Serial.println(ac4);
        Serial.print("ac5: "); Serial.println(ac5);
        Serial.print("ac6: "); Serial.println(ac6);
        Serial.print("b1: "); Serial.println(b1);
        Serial.print("b2: "); Serial.println(b2);
        Serial.print("mb: "); Serial.println(mb);
        Serial.print("mc: "); Serial.println(mc);
        Serial.print("md: "); Serial.println(md);
        */

        // Success!
        return (1);
    } else {
        // Error reading calibration data; bad component or connection?
        return (0);
    }
}


static char readInt(char address, int16_t *value)
// Read a signed integer (two bytes) from device
// address: register to start reading (plus subsequent register)
// value: external variable to store data (function modifies value)
{
    unsigned char data[2];

    data[0] = address;
    if (readBytes(data, 2)) {
        *value = (((int16_t)data[0] << 8) | (int16_t)data[1]);
        //if (*value & 0x8000) *value |= 0xFFFF0000; // sign extend if negative
        return (1);
    }
    *value = 0;
    return (0);
}


static char readUInt(char address, uint16_t *value)
// Read an unsigned integer (two bytes) from device
// address: register to start reading (plus subsequent register)
// value: external variable to store data (function modifies value)
{
    unsigned char data[2];

    data[0] = address;
    if (readBytes(data, 2)) {
        *value = (((uint16_t)data[0] << 8) | (uint16_t)data[1]);
        return (1);
    }
    *value = 0;
    return (0);
}


static char readBytes(unsigned char *values, char length)
// Read an array of bytes from device
// values: external array to hold data. Put starting register in values[0].
// length: number of bytes to read
{
    I2C_Lib_MasterRW(I2C_OP_Write, BMP180_ADDR, 1, values);
    I2C_Lib_MasterRW(I2C_OP_Read, BMP180_ADDR, length, values);
    I2C_Lib_WaitForRWDone();
    return (1);
}


static char writeBytes(unsigned char *values, char length)
// Write an array of bytes to device
// values: external array of data to write. Put starting register in values[0].
// length: number of bytes to write
{
    I2C_Lib_MasterRW(I2C_OP_Write, BMP180_ADDR, length, values);
    I2C_Lib_WaitForRWDone();
    return (1);
}

static int32_t computeB5(int32_t UT)
{
    int32_t X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) >> 15;
    int32_t X2 = ((int32_t)mc << 11) / (X1 + (int32_t)md);
    return X1 + X2;
}

char BMP180_StartTemperature(void)
// Begin a temperature reading.
// Will return delay in ms to wait, or 0 if I2C error
{
    unsigned char data[2], result;

    data[0] = BMP180_REG_CONTROL;
    data[1] = BMP180_COMMAND_TEMPERATURE;
    result = writeBytes(data, 2);
    if (result) // good write?
        return (5); // return the delay in ms (rounded up) to wait before retrieving data
    else
        return (0); // or return 0 if there was a problem communicating with the BMP
}


char BMP180_GetTemperature(double *T)
// Retrieve a previously-started temperature reading.
// Requires begin() to be called once prior to retrieve calibration parameters.
// Requires startTemperature() to have been called prior and sufficient time elapsed.
// T: external variable to hold result.
// Returns 1 if successful, 0 if I2C error.
{
    unsigned char data[2];
    char result;
    int32_t tu, B5;

    data[0] = BMP180_REG_RESULT;

    result = readBytes(data, 2);
    if (result) { // good read, calculate temperature
        tu = ((uint16_t)data[0] << 8) | data[1];

        //example from Bosch datasheet
        //tu = 27898;

        //example from http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf
        //tu = 0x69EC;

        B5 = computeB5(tu);
        *T = (B5 + 8) >> 4;
        *T /= 10;

        /*
        Serial.println();
        Serial.print("tu: "); Serial.println(tu);
        Serial.print("a: "); Serial.println(a);
        Serial.print("T: "); Serial.println(*T);
        */
    }
    return (result);
}


char BMP180_StartPressure(char oversampling)
// Begin a pressure reading.
// Oversampling: 0 to 3, higher numbers are slower, higher-res outputs.
// Will return delay in ms to wait, or 0 if I2C error.
{
    unsigned char data[2], result, delay;

    data[0] = BMP180_REG_CONTROL;

    switch (oversampling) {
    case 0:
        data[1] = BMP180_COMMAND_PRESSURE0;
        delay = 5;
        break;
    case 1:
        data[1] = BMP180_COMMAND_PRESSURE1;
        delay = 8;
        break;
    case 2:
        data[1] = BMP180_COMMAND_PRESSURE2;
        delay = 14;
        break;
    case 3:
        data[1] = BMP180_COMMAND_PRESSURE3;
        delay = 26;
        break;
    default:
        data[1] = BMP180_COMMAND_PRESSURE0;
        delay = 5;
        break;
    }
    oss = (data[1]>>6)&0x3;
    result = writeBytes(data, 2);
    if (result) // good write?
        return (delay); // return the delay in ms (rounded up) to wait before retrieving data
    else
        return (0); // or return 0 if there was a problem communicating with the BMP
}


char BMP180_GetPressure(double *P, double *T)
// Retrieve a previously started pressure reading, calculate abolute pressure in mbars.
// Requires begin() to be called once prior to retrieve calibration parameters.
// Requires startPressure() to have been called prior and sufficient time elapsed.
// Requires recent temperature reading to accurately calculate pressure.

// P: external variable to hold pressure.
// T: previously-calculated temperature.
// Returns 1 for success, 0 for I2C error.

// Note that calculated pressure value is absolute mbars, to compensate for altitude call sealevel().
{
    unsigned char data[3];
    char result;
    int32_t UP, B3, B5, B6, X1, X2, X3, p;
    uint32_t B4, B7;

    B5 = ((int)(*T * 10) << 4) - 8;

    data[0] = BMP180_REG_RESULT;

    result = readBytes(data, 3);
    if (result) { // good read, calculate pressure
        UP = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | (data[2]);
        UP >>= (8 - oss);

        //example from Bosch datasheet
        //pu = 23843;

        //example from http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf, pu = 0x982FC0;
        //pu = (0x98 * 256.0) + 0x2F + (0xC0/256.0);

        // do pressure calcs
        B6 = B5 - 4000;
        X1 = ((int32_t)b2 * ( (B6 * B6) >> 12 )) >> 11;
        X2 = ((int32_t)ac2 * B6) >> 11;
        X3 = X1 + X2;
        B3 = ((((int32_t)ac1 * 4 + X3) << oss) + 2) / 4;

        X1 = ((int32_t)ac3 * B6) >> 13;
        X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
        X3 = ((X1 + X2) + 2) >> 2;
        B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
        B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> oss );


        if (B7 < 0x80000000) {
            p = (B7 * 2) / B4;
        } else {
            p = (B7 / B4) * 2;
        }
        X1 = (p >> 8) * (p >> 8);
        X1 = (X1 * 3038) >> 16;
        X2 = (-7357 * p) >> 16;

        *P = p + ((X1 + X2 + (int32_t)3791) >> 4);

    }
    return (result);
}


double BMP180_Altitude(double P, double P0)
// Given a pressure measurement P (mb) and the pressure at a baseline P0 (mb),
// return altitude (meters) above baseline.
{
    return (44330.0 * (1 - pow(P / P0, 1 / 5.255)));
}

