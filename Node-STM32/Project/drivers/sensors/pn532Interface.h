

#ifndef __PN532_INTERFACE_H__
#define __PN532_INTERFACE_H__

#include "common.h"

#define PN532_PREAMBLE                (0x00)
#define PN532_STARTCODE1              (0x00)
#define PN532_STARTCODE2              (0xFF)
#define PN532_POSTAMBLE               (0x00)

#define PN532_HOSTTOPN532             (0xD4)
#define PN532_PN532TOHOST             (0xD5)

#define PN532_ACK_WAIT_TIME           (10)  // ms, timeout of waiting for ACK

#define PN532_INVALID_ACK             (-1)
#define PN532_TIMEOUT                 (-2)
#define PN532_INVALID_FRAME           (-3)
#define PN532_NO_SPACE                (-4)

#define REVERSE_BITS_ORDER(b)         b = (b & 0xF0) >> 4 | (b & 0x0F) << 4; \
                                      b = (b & 0xCC) >> 2 | (b & 0x33) << 2; \
                                      b = (b & 0xAA) >> 1 | (b & 0x55) << 1

#ifdef PN532_DEBUG
#define PN532_DBG_MSG(...)            DBG_MSG(__VA_ARGS__)
#define DMSG_HEX(x)                   printf("[0x%02x]",(x))
#define DMSG_INT(x)                   printf("%d",(x))
#else
#define PN532_DBG_MSG(...)            ((void)(0))
#define DMSG_INT(x)                   ((void)(0))
#define DMSG_HEX(x)                   ((void)(0))
#endif

struct PN532Interface
{
    void (*begin)();
    void (*wakeup)();

    /**
    * @brief    write a command and check ack
    * @param    header  packet header
    * @param    hlen    length of header
    * @param    body    packet body
    * @param    blen    length of body
    * @return   0       success
    *           not 0   failed
    */
    int8_t (*writeCommand)(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen);

    /**
    * @brief    read the response of a command, strip prefix and suffix
    * @param    buf     to contain the response data
    * @param    len     lenght to read
    * @param    timeout max time to wait, 0 means no timeout
    * @return   >=0     length of response without prefix and suffix
    *           <0      failed to read response
    */
    int16_t (*readResponse)(uint8_t buf[], uint8_t len, uint16_t timeout);
};

#endif

