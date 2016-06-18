#ifndef PIN_H__
#define PIN_H__

//I2C1_DevStructure is defined in CPAL
//All sensors with i2c interface should be connected to I2C1
#define I2C_HOST_DEV I2C1_DevStructure

#define RELAY_PORTS  {GPIOA, GPIOB, GPIOB, GPIOA}
#define RELAY_PINS   {GPIO_Pin_0, GPIO_Pin_10, GPIO_Pin_11, GPIO_Pin_7}
#define RELAY_ACTIVE_VAL   Bit_SET

#define PWM_OUT_PORT   GPIOA
#define PWM_OUT_PIN    GPIO_Pin_7
#define PWM_OUT_TIM    TIM3
#define PWM_OUT_CH     2
#define PWM_FREQ_HERTZ 50 

#define PN532_HSU_USART USART3
#define PN532_HSU_IRQ   USART3_IRQn

#define LT211_TXE_PORT GPIOA
#define LT211_TXE_PIN  GPIO_Pin_7
#define LT211_USART USART3
#define LT211_IRQ   USART3_IRQn
#define LT211_BAUD  4800
#define LT211_ADDR  1

#define BMP180_PORT    GPIOB
#define BMP180_SDA_PIN GPIO_Pin_7
#define BMP180_SCL_PIN GPIO_Pin_6

#define BH1750_PORT    GPIOB
#define BH1750_SDA_PIN GPIO_Pin_7
#define BH1750_SCL_PIN GPIO_Pin_6

#define DHT11_PORT GPIOA
#define DHT11_PIN  GPIO_Pin_1

#define DS18B20_PORT GPIOA
#define DS18B20_PIN  GPIO_Pin_1

#define SR501_PORT GPIOA
#define SR501_PIN  GPIO_Pin_0

#define MQ2_ADC_CHANNEL 4

#define GENERAL_ANALOG_ADC_CHANNEL 4

#define IR_EMIT_PORT GPIOA
#define IR_EMIT_PIN  GPIO_Pin_6

#define IR_RECV_PORT GPIOA
#define IR_RECV_PIN  GPIO_Pin_0

#define IRDA_PORT_IO        GPIOB
#define IRDA_PORT_BUSY      GPIOA
#define IRDA_SCL_PIN        GPIO_Pin_6
#define IRDA_SDA_PIN        GPIO_Pin_7
#define IRDA_BUSY_PIN       GPIO_Pin_5

#define EEP24Cxx_PORT    GPIOB
#define EEP24Cxx_SDA_PIN GPIO_Pin_7
#define EEP24Cxx_SCL_PIN GPIO_Pin_6

#define LED_BRD_PORT  GPIOB
#define LED_BRD_B_PIN GPIO_Pin_9
#define LED_BRD_G_PIN GPIO_Pin_8

#define KEY_BRD_PORT  GPIOB
#define KEY_BRD_PIN   GPIO_Pin_0

#define USART_DBG     USART1

#define ESP8266_IO_PORT GPIOC
#define ESP8266_IO0_PIN GPIO_Pin_13
#define ESP8266_IO2_PIN GPIO_Pin_14
#define ESP8266_RST_PIN GPIO_Pin_15
#define ESP8266_USART   USART2
#define ESP8266_USART_IRQ   USART2_IRQn

#define USB_DET_PORT GPIOB
#define USB_DET_PIN  GPIO_Pin_5

#endif
