#include "common.h"
#include "esp8266.h"
#include "usart.h"
#include "usbcdc.h"
#include "systick.h"
#include "iot_node.h"
#include "func.h"

#define ESP8266_LUA_CMD(format, ...) do{\
    USART_printf(ESP8266_USART, format "\r", ##__VA_ARGS__);\
    Delay_ms(300);\
}while(0)

static bool CDC_Forwarding;
static volatile bool wifi_connected;
static volatile bool mqtt_connected;
static volatile bool esp8266_stated;
static volatile bool last_cmd_result;
static volatile bool initialize_done;

static void RedirectUSBToESP8266(uint8_t* data_buffer, uint8_t Nb_bytes)
{
    for (int i = 0; i < Nb_bytes; ++i)
    {
        USART_putchar(ESP8266_USART, data_buffer[i]);
    }
}

static void gotResponse(char *token, char *param)
{
    DBG_MSG("[[%s]][[%s]]", token, param);
    if(strcmp(token, "control") == 0)
        IoTNode_HandleControl(param);
    if(strcmp(token, "connected") == 0)
        mqtt_connected = true;
    else if(strcmp(token, "offline") == 0)
        mqtt_connected = false;
    else if(strcmp(token, "wifi") == 0){
        if(*param == '5')
            wifi_connected = true;
    }else if(strcmp(token, "started") == 0)
        esp8266_stated = true;
    else if(strcmp(token, "yes") == 0)
        last_cmd_result = true;
    else if(strcmp(token, "no") == 0)
        last_cmd_result = false;
    else if(strcmp(token, "initialized") == 0)
        initialize_done = true;

}

static void parse8266Output(uint8_t in)
{
    enum{STATE_WAIT, STATE_TOKEN, STATE_PARAM};
    static uint8_t state = STATE_WAIT;
    static int tokenLen, paramLen;
    static char tokenBuf[16], paramBuf[16];
    switch(state){
    case STATE_WAIT:
        if(in == '#'){
            tokenLen = 0;
            state = STATE_TOKEN;
        }
        break;
    case STATE_TOKEN:
        if(in == '+' || in == '\r' || in == '\n'){
            tokenBuf[tokenLen] = '\0';
            if(in == '+'){
                paramLen = 0;
                state = STATE_PARAM;
            }else{
                gotResponse(tokenBuf, NULL);
                state = STATE_WAIT;
            }
        }else if(tokenLen+1 < sizeof(tokenBuf)){
            tokenBuf[tokenLen++] = in;
        }
        break;
    case STATE_PARAM:
        if(in == '\r' || in == '\n'){
            paramBuf[paramLen] = '\0';
            gotResponse(tokenBuf, paramBuf);
            state = STATE_WAIT;
        }else if(paramLen+1 < sizeof(paramBuf)){
            paramBuf[paramLen++] = in;
        }
        break;

    }
}

bool ESP8266_UpgradeModeDetected()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_GPIOClockCmd(ESP8266_IO_PORT, ENABLE);

    GPIO_InitStructure.GPIO_Pin = ESP8266_IO0_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(ESP8266_IO_PORT, &GPIO_InitStructure);

    return GPIO_ReadInputDataBit(ESP8266_IO_PORT, ESP8266_IO0_PIN) == RESET;
}

void ESP8266_Init(bool upgrade_baud)
{
    CDC_Forwarding = false;
    USARTx_Config(ESP8266_USART, upgrade_baud ? 115200 : 9600);
    USART_RxInt_Config(true, ESP8266_USART, ESP8266_USART_IRQ);
}

void ESP8266_Enable_CDC_Forwarding()
{
    USBCDC_SetReceiveCallback(RedirectUSBToESP8266);
    CDC_Forwarding = true;
}

void ESP8266_USART_IT_Handler()
{
    if(USART_GetITStatus(ESP8266_USART, USART_IT_RXNE) == SET){
        uint8_t byte = USART_ReceiveData(ESP8266_USART);
        if(CDC_Forwarding)
            USBCDC_SendByte(byte);
        parse8266Output(byte);
    }

}

void ESP8266_CheckWifiState()
{
    ESP8266_LUA_CMD("print('\\035wifi+'..wifi.sta.status())");
}

bool ESP8266_IsStarted()
{
    return esp8266_stated;
} 

bool ESP8266_IsWifiConnected()
{
    return wifi_connected;
}

void ESP8266_InitMqtt(char *name)
{
    ESP8266_LUA_CMD("c=require('comm');c.init('%s')", name);
}

void ESP8266_InitMqttWithChipID()
{
    ESP8266_LUA_CMD("c=require('comm');");
    Delay_ms(50);
    ESP8266_LUA_CMD("c.init('i_' .. node.chipid())");
}

void ESP8266_MqttConnect(char *ip, int port)
{
    //connect is async
    ESP8266_LUA_CMD("c.connect('%s',%d)", ip, port);
}

bool ESP8266_IsMqttConnected()
{
    return mqtt_connected;
}

void ESP8266_MqttPublishValue(char *key, char *value)
{
    ESP8266_LUA_CMD("c.publish('values',[[%s]],[[%s]],1,1)", key, value);
}

void ESP8266_MqttPublishEvent(char *key, char *value)
{
    ESP8266_LUA_CMD("c.publish('events',[[%s]],[[%s]],2,0)", key, value);
}

void ESP8266_ReportCapability(char *type, char *value)
{
    ESP8266_LUA_CMD("c.publish('capability','%s','%s',1,1)", type, value);
}

void ESP8266_Restart(void)
{
    ESP8266_LUA_CMD("node.restart()");
    wifi_connected = mqtt_connected = false;
    esp8266_stated = initialize_done = false;
    Delay_ms(2000); //Wait for restarting
}

bool ESP8266_CheckLuaScripts(void)
{
    ESP8266_LUA_CMD("print((file.list())['init.lua']~=nil and'\\035yes'or'\\035no')");
    Delay_ms(500); //Wait for command result
    return last_cmd_result;
}

void ESP8266_SetWiFiCredentials(void)
{
    ESP8266_LUA_CMD("wifi.sta.config([[%s]],[[%s]])", ROUTER_SSID, ROUTER_PASSWD);
}

void ESP8266_InitializeLuaScripts(void)
{
    // ESP8266_LUA_CMD("file.remove('init.lua')");
    // Delay_ms(200); //Extra delay for flash
    ESP8266_LUA_CMD("wifi.setmode(wifi.STATION)");
    ESP8266_SetWiFiCredentials();
    Delay_ms(200); //Extra delay for flash
    ESP8266_LUA_CMD("wifi.sta.autoconnect(1)");
    do {
        ESP8266_CheckWifiState();
        Delay_ms(1000);
    } while (!ESP8266_IsWifiConnected());

    initialize_done = false;

    // bootstrap from HTTP
    // ESP8266_LUA_CMD("_,_,gw=wifi.sta.getip();sk=net.createConnection(net.TCP, 0);");
    // ESP8266_LUA_CMD("sk:on('receive',function(sck,c)print(c)end)");
    // ESP8266_LUA_CMD("sk:on('connection',function(sck)sk:send('GET /iot-bootstrap HTTP/1.0\\r\\nConnection: close\\r\\n\\r\\n')end)");
    // ESP8266_LUA_CMD("sk:connect(80,gw)");

    // bootstrap from MQTT
    ESP8266_LUA_CMD("m=mqtt.Client('init'..node.chipid())");
    ESP8266_LUA_CMD("m:on('message', function(conn, topic, data)pcall(loadstring(data));end)");
    ESP8266_LUA_CMD("m:connect('%s',%d,0,function(conn)m:subscribe('/bootstrap/script',2,function(conn)print('subscribed');end)end)",
        MQTT_BROKER_IP, MQTT_BROKER_PORT);

    while(!initialize_done);
}
