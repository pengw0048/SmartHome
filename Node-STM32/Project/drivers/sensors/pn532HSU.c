
#include "pn532HSU.h"
#include "usart.h"
#include "common.h"
#include "systick.h"
#include "circular_buffer.h"
#include <string.h>

static uint8_t command = 0;
static struct CircularBuffer* serialBuf;

static int8_t receive(uint8_t *buf, int len, uint16_t timeout);
static int8_t readAckFrame(void);
static int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout);

static int readByte(void)
{
    uint8_t b;
    if(!CircularBuffer_Pop(serialBuf, &b))
        return -1;
    return b;
}

static void dumpBuf(void)
{
    if(CircularBuffer_Size(serialBuf)){
        DBG_MSG("Dump serial buffer: ");
        while(CircularBuffer_Size(serialBuf)){
            uint8_t ret = readByte();
            printf("%02x",ret);
        }
        printf("\n");
    }
}

void PN532_USART_IT_Handler(void)
{
    if(USART_GetITStatus(PN532_HSU_USART, USART_IT_RXNE) != RESET){
        uint8_t byte = USART_getchar(PN532_HSU_USART);
        CircularBuffer_Push(serialBuf, byte);
    }
}

static void begin()
{
    serialBuf = CircularBuffer_New(128);
    if(!serialBuf)
        return;
    USARTx_Config(PN532_HSU_USART,115200);
    USART_RxInt_Config(true, PN532_HSU_USART, PN532_HSU_IRQ);
}

static void wakeup()
{
    USART_putchar(PN532_HSU_USART,0x55);
    USART_putchar(PN532_HSU_USART,0x55);
    USART_putchar(PN532_HSU_USART,0);
    USART_putchar(PN532_HSU_USART,0);
    USART_putchar(PN532_HSU_USART,0);

    /** dump serial buffer */
    dumpBuf();

}

static int8_t writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{

    /** dump serial buffer */
    dumpBuf();

    command = header[0];
    
    USART_putchar(PN532_HSU_USART,PN532_PREAMBLE);
    USART_putchar(PN532_HSU_USART,PN532_STARTCODE1);
    USART_putchar(PN532_HSU_USART,PN532_STARTCODE2);
    
    uint8_t length = hlen + blen + 1;   // length of data field: TFI + DATA
    USART_putchar(PN532_HSU_USART,length);
    USART_putchar(PN532_HSU_USART,~length + 1);         // checksum of length
    
    USART_putchar(PN532_HSU_USART,PN532_HOSTTOPN532);
    uint8_t sum = PN532_HOSTTOPN532;    // sum of TFI + DATA

    // DBG_MSG("Write: ");
    
    USART_write(PN532_HSU_USART,header, hlen);
    for (uint8_t i = 0; i < hlen; i++) {
        sum += header[i];

        // DMSG_HEX(header[i]);
    }

    USART_write(PN532_HSU_USART,body, blen);
    for (uint8_t i = 0; i < blen; i++) {
        sum += body[i];

        // DMSG_HEX(body[i]);
    }
    
    uint8_t checksum = ~sum + 1;            // checksum of TFI + DATA
    USART_putchar(PN532_HSU_USART,checksum);
    USART_putchar(PN532_HSU_USART,PN532_POSTAMBLE);

    return readAckFrame();
}

static int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint8_t tmp[3];
    
    // DBG_MSG("Read:  ");
    
    /** Frame Preamble and Start Code */
    if(receive(tmp, 3, timeout)<=0){
        return PN532_TIMEOUT;
    }
    if(0 != tmp[0] || 0!= tmp[1] || 0xFF != tmp[2]){
        DBG_MSG("Preamble error");
        return PN532_INVALID_FRAME;
    }
    
    /** receive length and check */
    uint8_t length[2];
    if(receive(length, 2, timeout) <= 0){
        return PN532_TIMEOUT;
    }
    if( 0 != (uint8_t)(length[0] + length[1]) ){
        DBG_MSG("Length error");
        return PN532_INVALID_FRAME;
    }
    length[0] -= 2;
    if( length[0] > len){
        return PN532_NO_SPACE;
    }
    
    /** receive command byte */
    uint8_t cmd = command + 1;               // response command
    if(receive(tmp, 2, timeout) <= 0){
        return PN532_TIMEOUT;
    }
    if( PN532_PN532TOHOST != tmp[0] || cmd != tmp[1]){
        DBG_MSG("Command error");
        return PN532_INVALID_FRAME;
    }
    
    if(receive(buf, length[0], timeout) != length[0]){
        return PN532_TIMEOUT;
    }
    uint8_t sum = PN532_PN532TOHOST + cmd;
    for(uint8_t i=0; i<length[0]; i++){
        sum += buf[i];
    }
    
    /** checksum and postamble */
    if(receive(tmp, 2, timeout) <= 0){
        return PN532_TIMEOUT;
    }
    if( 0 != (uint8_t)(sum + tmp[0]) || 0 != tmp[1] ){
        DBG_MSG("Checksum error");
        return PN532_INVALID_FRAME;
    }
    
    return length[0];
}

static int8_t readAckFrame()
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
    uint8_t ackBuf[sizeof(PN532_ACK)];
    
    // DBG_MSG("Ack: ");
    
    if( receive(ackBuf, sizeof(PN532_ACK), PN532_ACK_WAIT_TIME) <= 0 ){
        DBG_MSG("ACK Timeout");
        return PN532_TIMEOUT;
    }
    
    if( memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK)) ){
        DBG_MSG("ACK Invalid");
        return PN532_INVALID_ACK;
    }
    return 0;
}

/**
    @brief receive data .
    @param buf --> return value buffer.
           len --> length expect to receive.
           timeout --> time of reveiving
    @retval number of received bytes, 0 means no data received.
*/
static int8_t receive(uint8_t *buf, int len, uint16_t timeout)
{
  int read_bytes = 0;
  int ret;
  SysTick_t start_millis;
  
  while (read_bytes < len) {
    start_millis = GetSystemTick();
    do {
      ret = readByte();
      if (ret >= 0) {
        break;
     }
    } while((timeout == 0) || ((GetSystemTick()- start_millis ) < timeout));
    
    if (ret < 0) {
        if(read_bytes){
            return read_bytes;
        }else{
            return PN532_TIMEOUT;
        }
    }
    buf[read_bytes] = (uint8_t)ret;
    // DMSG_HEX(ret);
    read_bytes++;
  }
  return read_bytes;
}

struct PN532Interface PN532_HSU = {
    .begin = begin,
    .wakeup = wakeup,
    .writeCommand = writeCommand,
    .readResponse = readResponse
};
