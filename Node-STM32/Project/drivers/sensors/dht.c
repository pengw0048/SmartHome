/* DHT library
MIT license
written by Adafruit Industries
ported bt YuXiang Zhang
*/

#include "dht.h"
#include "common.h"
#include "systick.h"
#include <math.h>

static uint8_t data[6];
static uint8_t _type;
static SysTick_t _lastreadtime;
static bool firstreading;
static GPIO_InitTypeDef GPIO_InitStructure;

static bool DHT_read(void);

static void DHT_pinMode(GPIOMode_TypeDef mode)
{
    GPIO_InitStructure.GPIO_Mode = mode;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

void DHT_Init(uint8_t type)
{
    _type = type;
    firstreading = true;

    RCC_GPIOClockCmd(DHT11_PORT, ENABLE);
    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    // set up the pins!
    DHT_pinMode(GPIO_Mode_IPU);
    _lastreadtime = 0;
}

//boolean S == Scale.  True == Farenheit; False == Celcius
float DHT_ReadTemperature()
{
    bool S = false;
    float f;

    if (DHT_read()) {
        switch (_type) {
        case DHT11:
            f = data[2];
            if (S)
                f = DHT_ConvertCtoF(f);

            return f;
        case DHT22:
        case DHT21:
            f = data[2] & 0x7F;
            f *= 256;
            f += data[3];
            f /= 10;
            if (data[2] & 0x80)
                f *= -1;
            if (S)
                f = DHT_ConvertCtoF(f);

            return f;
        }
    }
    return NAN;
}

float DHT_ConvertCtoF(float c)
{
    return c * 9 / 5 + 32;
}

float DHT_ConvertFtoC(float f)
{
    return (f - 32) * 5 / 9;
}

float DHT_ReadHumidity(void)
{
    float f;
    if (DHT_read()) {
        switch (_type) {
        case DHT11:
            f = data[0];
            return f;
        case DHT22:
        case DHT21:
            f = data[0];
            f *= 256;
            f += data[1];
            f /= 10;
            return f;
        }
    }
    return NAN;
}

float DHT_ComputeHeatIndex(float tempFahrenheit, float percentHumidity)
{
    // Adapted from equation at: https://github.com/adafruit/DHT-sensor-library/issues/9 and
    // Wikipedia: http://en.wikipedia.org/wiki/Heat_index
    return -42.379 +
           2.04901523 * tempFahrenheit +
           10.14333127 * percentHumidity +
           -0.22475541 * tempFahrenheit * percentHumidity +
           -0.00683783 * pow(tempFahrenheit, 2) +
           -0.05481717 * pow(percentHumidity, 2) +
           0.00122874 * pow(tempFahrenheit, 2) * percentHumidity +
           0.00085282 * tempFahrenheit * pow(percentHumidity, 2) +
           -0.00000199 * pow(tempFahrenheit, 2) * pow(percentHumidity, 2);
}


static bool DHT_read(void)
{
    uint8_t laststate = 1;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    SysTick_t currenttime;

    // Check if sensor was read less than two seconds ago and return early
    // to use last reading.
    currenttime = GetSystemTick();
    if (currenttime < _lastreadtime) {
        // ie there was a rollover
        _lastreadtime = 0;
    }
    if (!firstreading && ((currenttime - _lastreadtime) < 2000)) {
        return true; // return last correct measurement
        //delay(2000 - (currenttime - _lastreadtime));
    }
    firstreading = false;
    /*
      Serial.print("Currtime: "); Serial.print(currenttime);
      Serial.print(" Lasttime: "); Serial.print(_lastreadtime);
    */
    _lastreadtime = GetSystemTick();

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    // pull the pin high and wait 250 milliseconds
    // DHT_pinMode(GPIO_Mode_IPU);
    // Delay_ms(250);

    // now pull it low for ~20 milliseconds
    DHT_pinMode(GPIO_Mode_Out_PP);
    GPIO_ResetBits(DHT11_PORT, DHT11_PIN);
    Delay_ms(20);

    __disable_irq();
    // GPIO_SetBits(DHT11_PORT, DHT11_PIN);
    // Delay_us(40);
    DHT_pinMode(GPIO_Mode_IPU);

    // uint8_t max_counter = 0, min_counter = 200;
    // read in timings
    for ( i = 0; i < MAXTIMINGS; i++) {
        counter = 0;
        while (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) == laststate) {
            counter++;
            Delay_us(3);
            if (counter == 255) {
                break;
            }
        }
        laststate = GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN);

        if (counter == 255) break;

        // ignore first 3 transitions
        if ((i >= 4) && ((i & 1) == 0)) {
            // shove each bit into the storage bytes
            data[j / 8] <<= 1;
            if (counter > 8)
                data[j / 8] |= 1;
            j++;
            // if(counter > max_counter)
            //   max_counter = counter;
            // if(counter < min_counter)
            //   min_counter = counter;
        }

    }

    __enable_irq();
    // DBG_MSG("Counter: %d %d", min_counter, max_counter);

    /*
    Serial.println(j, DEC);
    Serial.print(data[0], HEX); Serial.print(", ");
    Serial.print(data[1], HEX); Serial.print(", ");
    Serial.print(data[2], HEX); Serial.print(", ");
    Serial.print(data[3], HEX); Serial.print(", ");
    Serial.print(data[4], HEX); Serial.print(" =? ");
    Serial.println(data[0] + data[1] + data[2] + data[3], HEX);
    */

    // check we read 40 bits and that the checksum matches
    if ((j >= 40) &&
            (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
        return true;
    }


    return false;

}