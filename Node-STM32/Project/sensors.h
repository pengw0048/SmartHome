#ifndef SENSORS_H__
#define SENSORS_H__

#include "systick.h"

enum {SENSOR_VALUE_INT, SENSOR_VALUE_FLOAT, SENSOR_VALUE_BOOL, SENSOR_VALUE_STRING};
enum {
    SENSOR_FLAG_INITIALIZED = 1, 
    SENSOR_PUBLISH_CHANGES_ONLY = 2,
};

union sensor_value_t{
    int value_int;
    float value_float;
    bool value_bool;
    char* value_string;
};
struct sensor_t {
    char model[8];
    char input_name[16]; //"temperature", "humidity", etc.
    char unit[4]; //"C", "Pa", etc.
    uint8_t value_type;
    uint8_t flags;
    uint8_t reserved[2];
    union sensor_value_t value;
    SysTick_t latest_sample;
    SysTick_t sample_rate;
    bool (*driver_init)(struct sensor_t *);
    bool (*measure)(struct sensor_t *);
};

extern const int sensors_count;
extern struct sensor_t **sensors;

#endif
