#ifndef ACTUATOR_H__
#define ACTUATOR_H__

#include "systick.h"
#include "common.h"

enum {ACTUATOR_VALUE_BOOL, ACTUATOR_VALUE_INT, ACTUATOR_VALUE_FLOAT};
enum {
    ACTUATOR_FLAG_INITIALIZED = 1, 
    ACTUATOR_FLAG_TRIGGER = 2,
};
union actuator_value_t{
    bool value_bool;
    int value_int;
    float value_float;
};
struct actuator_t {
    char actuator_name[16]; //"switch", "IR", etc.
    uint8_t value_type;
    uint8_t flags;
    uint8_t reserved[2];
    union actuator_value_t old_value, value;
    SysTick_t latest_update;
    bool (*driver_init)(struct actuator_t *);
    bool (*action)(struct actuator_t *);
};

extern struct actuator_t **actuators;
extern const int actuators_count;

#endif
