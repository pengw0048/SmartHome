#ifndef ESP8266_H__
#define ESP8266_H__


void ESP8266_Init(bool upgrade_baud);
void ESP8266_Enable_CDC_Forwarding(void);
bool ESP8266_IsStarted(void);
void ESP8266_CheckWifiState(void);
bool ESP8266_IsWifiConnected(void);
void ESP8266_InitMqttWithChipID();
void ESP8266_InitMqtt(char *name);
void ESP8266_MqttConnect(char *ip, int port);
bool ESP8266_IsMqttConnected();
void ESP8266_MqttPublishValue(char *key, char *value);
void ESP8266_MqttPublishEvent(char *key, char *value);
void ESP8266_ReportCapability(char *type, char *value);

bool ESP8266_UpgradeModeDetected(void);
void ESP8266_Restart(void);
bool ESP8266_CheckLuaScripts(void);
void ESP8266_SetWiFiCredentials(void);
void ESP8266_InitializeLuaScripts(void);

void ESP8266_USART_IT_Handler(void);

#endif