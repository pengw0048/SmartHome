#include "iot_node.h"
#include "common.h"
#include "esp8266.h"
#include "sensors.h"
#include "actuators.h"
#include "systick.h"
#include "func.h"
#include <string.h>
#include <stdlib.h>

static volatile char control_buf[16];

static void initSensors()
{
    for (int i = 0; i < sensors_count; ++i) {
        if (sensors[i]->driver_init(sensors[i])) {
            sensors[i]->flags |= SENSOR_FLAG_INITIALIZED;
        } else {
            ERR_MSG("Initialization of sensor %s failed",
                    sensors[i] -> model);
        }
    }
}

static void initActuators()
{
    for (int i = 0; i < actuators_count; ++i) {
        if(actuators[i]->driver_init(actuators[i])) {
            actuators[i]->flags |= ACTUATOR_FLAG_INITIALIZED;
        } else {
            ERR_MSG("Initialization of actuator %s failed",
                    actuators[i] -> actuator_name);
        }
    }
}

static void publishMeasurement(struct sensor_t *s)
{
    static char value_buf[16];
    char* pvalue = value_buf;
    if(!ESP8266_IsMqttConnected())
        return;
    if (s->value_type == SENSOR_VALUE_INT)
        snprintf(value_buf, sizeof(value_buf), "%d %s",
                 s->value.value_int,
                 s->unit);
    else if (s->value_type == SENSOR_VALUE_FLOAT)
        snprintf(value_buf, sizeof(value_buf), "%.2f %s",
                 s->value.value_float,
                 s->unit);
    else if (s->value_type == SENSOR_VALUE_BOOL)
        snprintf(value_buf, sizeof(value_buf), "%s %s",
                 s->value.value_bool ? "true" : "false",
                 s->unit);
    else if (s->value_type == SENSOR_VALUE_STRING)
        pvalue = s->value.value_string;
    else {
        ERR_MSG("Unknown value type of %s", s->model);
        return;
    }
    DBG_MSG("%s-%s: %s", s->model, s->input_name, pvalue);
    ESP8266_MqttPublishValue(s->input_name, pvalue);
}

static void updateMeasurement()
{
    for (int i = 0; i < sensors_count; ++i) {
        SysTick_t now = GetSystemTick();
        if (!sensors[i]->flags & SENSOR_FLAG_INITIALIZED)
            continue;
        if (now - sensors[i]->latest_sample < sensors[i]->sample_rate)
            continue;
        union sensor_value_t saved_value = sensors[i]->value;
        if (sensors[i]->measure(sensors[i])) {
            sensors[i]->latest_sample = now;
            if(sensors[i]->flags & SENSOR_PUBLISH_CHANGES_ONLY){
                if(memcmp(&saved_value, &sensors[i]->value, sizeof(saved_value)))
                    publishMeasurement(sensors[i]);
            }else{
                publishMeasurement(sensors[i]);
            }
        } else {
            // ERR_MSG("Measuring %s on %s failed",
            //         sensors[i]->input_name,
            //         sensors[i]->model);
        }
    }
}

static void initNetwork(void)
{
    uint32_t id[3];
    char name_buf[32];
    SysTick_t tmr = GetSystemTick();
    
    while (!ESP8266_IsStarted()) {
        if(GetSystemTick()-tmr > ESP8266_BOOT_TIMEOUT) {
            ERR_MSG("ESP8266 booting timed out");
            if(ESP8266_CheckLuaScripts()){
                DBG_MSG("Restarting ESP8266...");
                ESP8266_Restart();
            }else{
                DBG_MSG("Lua scripts not found, initializing...");
                ESP8266_InitializeLuaScripts();
                ESP8266_Restart();
            }
            tmr = GetSystemTick();
        }
    }
    ESP8266_SetWiFiCredentials();
    DBG_MSG("ESP8266 started");
    do {
        ESP8266_CheckWifiState();
        Delay_ms(1000);
    } while (!ESP8266_IsWifiConnected());
    LED_GREEN(true);

    Chip_GetUniqueID(id);
    ESP8266_InitMqttWithChipID();
    ESP8266_MqttConnect(MQTT_BROKER_IP, MQTT_BROKER_PORT);
    while (!ESP8266_IsMqttConnected());
    DBG_MSG("MQTT connected");
    Delay_ms(1000);
}

static bool fetchActuatorValue(struct actuator_t* a, const char *value, bool* changed)
{
    union actuator_value_t newval;
    switch(a->value_type){
    case ACTUATOR_VALUE_BOOL:
        if(strcmp(value, "true") == 0)
            newval.value_bool = true;
        else if(strcmp(value, "false") == 0)
            newval.value_bool = false;
        else //invalid value
            return false;
        *changed = newval.value_bool != a->value.value_bool;
        break;
    case ACTUATOR_VALUE_INT:
        newval.value_int = atoi(value);
        *changed = newval.value_int != a->value.value_int;
        break;
    case ACTUATOR_VALUE_FLOAT:
        newval.value_float = atof(value);
        *changed = newval.value_float != a->value.value_float;
        break;
    }
    a->old_value = a->value;
    a->value = newval;
    return true;
}

static void doAction(char* ctrl)
{
    char* plus = strchr(ctrl, '+');
    if(!plus)
        return;
    *plus = '\0';
    for (int i = 0; i < actuators_count; ++i)
    {
        bool changed;
        struct actuator_t* a = actuators[i];
        if(strcmp(a->actuator_name, ctrl) == 0){
            DBG_MSG("found actuator %s", ctrl);

            if (!a->flags & ACTUATOR_FLAG_INITIALIZED) {
                ERR_MSG("actuator %s is not initialized", a->actuator_name);
                break;
            }
            if(fetchActuatorValue(a, plus+1, &changed)){
                a->latest_update = GetSystemTick();
                if(changed || (a->flags & ACTUATOR_FLAG_TRIGGER)){
                    a->action(a);
                }
            }
            break;
        }
    }
}

void reportNodeInfo(void)
{
    const char *ctrl_type;
    static char capability[512];
    ESP8266_MqttPublishEvent("online", "");
    Delay_ms(1000);
    capability[0] = '\0';
    for (int i = 0; i < sensors_count; ++i)
    {
        if (!sensors[i]->flags & SENSOR_FLAG_INITIALIZED)
            continue;
        strcat(capability, sensors[i]->input_name);
        strcat(capability, ",");
        strcat(capability, sensors[i]->unit);
        strcat(capability, "\\n");
    }
    if(capability[0] != '\0'){
        ESP8266_ReportCapability("values", capability);
        Delay_ms(1000);
    }

    capability[0] = '\0';
    for (int i = 0; i < actuators_count; ++i)
    {
        if (!actuators[i]->flags & ACTUATOR_FLAG_INITIALIZED)
            continue;
        strcat(capability, actuators[i]->actuator_name);
        strcat(capability, ",");
        switch(actuators[i]->value_type){
        case ACTUATOR_VALUE_BOOL:
            ctrl_type = "bool";
            break;
        case ACTUATOR_VALUE_INT:
            ctrl_type = "int";
            break;
        case ACTUATOR_VALUE_FLOAT:
            ctrl_type = "float";
            break;
        default:
            ctrl_type = "unknown";
            ERR_MSG("Unknown ctrl_type for %s", actuators[i]->actuator_name);
            break;
        }
        strcat(capability, ctrl_type);
        strcat(capability, "\\n");
    }
    if(capability[0] != '\0'){
        ESP8266_ReportCapability("control", capability);
    }
}

void IoTNode_HandleControl(const char* param)
{
    //temporarily store control command to avoid
    // spending too much time in interrupt context
    if(control_buf[0] == '\0'){
        strncpy(control_buf, param, sizeof(control_buf)-1);
        control_buf[sizeof(control_buf)-1] = '\0';
    }
}

void IoTNode_Begin()
{
    DBG_MSG("%d sensors defined", sensors_count);
    DBG_MSG("%d actuators defined", actuators_count);
    initActuators();
    initSensors();
    initNetwork();
    reportNodeInfo();
    while (1) {
        if(control_buf[0] != '\0'){
            doAction(control_buf);
            control_buf[0] = '\0';
        }
        updateMeasurement();
        if(!ESP8266_IsMqttConnected()){
            //Connection lost
            ERR_MSG("Connection Lost");
            ESP8266_Restart();
            initNetwork();
        }
    }
}