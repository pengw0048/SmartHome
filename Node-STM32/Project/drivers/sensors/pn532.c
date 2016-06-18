/**************************************************************************/
/*!
    @file     PN532.cpp
    @author   Adafruit Industries & Seeed Studio
    @license  BSD
*/
/**************************************************************************/

#include "common.h"
#include "pn532.h"
#include "pn532HSU.h"
#include "systick.h"
#include <string.h>



const uint16_t registerAddrForTypeB[12] = {
    PN53X_REG_CIU_Mode,
    PN53X_REG_CIU_TxAuto,
    PN53X_REG_CIU_TxMode,
    PN53X_REG_CIU_RxMode,
    PN53X_REG_CIU_TypeB,
    PN53X_REG_CIU_Demod,
    PN53X_REG_CIU_GsNOn,
    PN53X_REG_CIU_CWGsP,
    PN53X_REG_CIU_ModGsP,
    PN53X_REG_CIU_RxThreshold,
    PN53X_REG_CIU_ModWidth,
    PN53X_REG_CIU_ManualRCV,
};

const uint8_t registerValueForTypeB[12] = {
    0xff,
    0x00,
    0x03,
    0x03,
    0x03,
    0x4d,
    0xff,
    0x3f,
    0x18,
    0x4d,
    0x68,
    0x10,
};

enum {CRC_A = 1, CRC_B = 2};
static uint8_t _uid[7];  // ISO14443A uid
static uint8_t _uidLen;  // uid len
static uint8_t _key[6];  // Mifare Classic key
static uint8_t inListedTag; // Tg number of inlisted tag.

static uint8_t pn532_packetbuffer[64];

static uint8_t registerValueBackup[12];


static unsigned short UpdateCrc(unsigned char ch, unsigned short *lpwCrc);
static void ComputeCrc(uint8_t CRCType, uint8_t *Data, uint8_t Length, uint8_t *TransmitFirst, uint8_t *TransmitSecond);
static void setPN532RegsForTypeB(void);
static void restorePN532RegsForTypeB(void);
static bool doReadTsighuaStuCard(uint8_t cardId[3], uint8_t expire[3], char studentId[11]);


#define HAL(func)   (PN532_HSU.func)


/**************************************************************************/
/*!
    @brief  Setups the HW
*/
/**************************************************************************/
void PN532_begin()
{
    HAL(begin)();
    HAL(wakeup)();
}

/**************************************************************************/
/*!
    @brief  Prints a hexadecimal value in plain characters

    @param  data      Pointer to the uint8_t data
    @param  numBytes  Data length in bytes
*/
/**************************************************************************/
void PN532_PrintHex(const uint8_t *data, const uint32_t numBytes)
{
#ifdef ARDUINO
    for (uint8_t i = 0; i < numBytes; i++) {
        if (data[i] < 0x10) {
            Serial.print(" 0");
        } else {
            Serial.print(' ');
        }
        Serial.print(data[i], HEX);
    }
    Serial.println("");
#else
    for (uint8_t i = 0; i < numBytes; i++) {
        printf(" %2X", data[i]);
    }
    printf("\n");
#endif
}

/**************************************************************************/
/*!
    @brief  Prints a hexadecimal value in plain characters, along with
            the char equivalents in the following format

            00 00 00 00 00 00  ......

    @param  data      Pointer to the data
    @param  numBytes  Data length in bytes
*/
/**************************************************************************/
void PN532_PrintHexChar(const uint8_t *data, const uint32_t numBytes)
{
#ifdef ARDUINO
    for (uint8_t i = 0; i < numBytes; i++) {
        if (data[i] < 0x10) {
            Serial.print(" 0");
        } else {
            Serial.print(' ');
        }
        Serial.print(data[i], HEX);
    }
    Serial.print("    ");
    for (uint8_t i = 0; i < numBytes; i++) {
        char c = data[i];
        if (c <= 0x1f || c > 0x7f) {
            Serial.print('.');
        } else {
            Serial.print(c);
        }
    }
    Serial.println("");
#else
    for (uint8_t i = 0; i < numBytes; i++) {
        printf(" %2X", data[i]);
    }
    printf("    ");
    for (uint8_t i = 0; i < numBytes; i++) {
        char c = data[i];
        if (c <= 0x1f || c > 0x7f) {
            printf(".");
        } else {
            printf("%c", c);
        }
        printf("\n");
    }
#endif
}

/**************************************************************************/
/*!
    @brief  Checks the firmware version of the PN5xx chip

    @returns  The chip's firmware version and ID
*/
/**************************************************************************/
uint32_t PN532_getFirmwareVersion(void)
{
    uint32_t response;

    pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

    if (HAL(writeCommand)(pn532_packetbuffer, 1, 0, 0)) {
        return 0;
    }

    // read data packet
    int16_t status = HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000);
    if (0 > status) {
        return 0;
    }

    response = pn532_packetbuffer[0];
    response <<= 8;
    response |= pn532_packetbuffer[1];
    response <<= 8;
    response |= pn532_packetbuffer[2];
    response <<= 8;
    response |= pn532_packetbuffer[3];

    return response;
}


/**************************************************************************/
/*!
    Writes an 8-bit value that sets the state of the PN532's GPIO pins

    @warning This function is provided exclusively for board testing and
             is dangerous since it will throw an error if any pin other
             than the ones marked "Can be used as GPIO" are modified!  All
             pins that can not be used as GPIO should ALWAYS be left high
             (value = 1) or the system will become unstable and a HW reset
             will be required to recover the PN532.

             pinState[0]  = P30     Can be used as GPIO
             pinState[1]  = P31     Can be used as GPIO
             pinState[2]  = P32     *** RESERVED (Must be 1!) ***
             pinState[3]  = P33     Can be used as GPIO
             pinState[4]  = P34     *** RESERVED (Must be 1!) ***
             pinState[5]  = P35     Can be used as GPIO

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
bool PN532_writeGPIO(uint8_t pinstate)
{
    // Make sure pinstate does not try to toggle P32 or P34
    pinstate |= (1 << PN532_GPIO_P32) | (1 << PN532_GPIO_P34);

    // Fill command buffer
    pn532_packetbuffer[0] = PN532_COMMAND_WRITEGPIO;
    pn532_packetbuffer[1] = PN532_GPIO_VALIDATIONBIT | pinstate;  // P3 Pins
    pn532_packetbuffer[2] = 0x00;    // P7 GPIO Pins (not used ... taken by I2C)

    PN532_DBG_MSG("Writing P3 GPIO: ");
    DMSG_HEX(pn532_packetbuffer[1]);
    PN532_DBG_MSG("\n");

    // Send the WRITEGPIO command (0x0E)
    if (HAL(writeCommand)(pn532_packetbuffer, 3, 0, 0))
        return 0;

    return (0 < HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000));
}

bool PN532_writeGPIOP7(uint8_t pinstate)
{
    pinstate &= (1 << PN532_GPIO_P71) | (1 << PN532_GPIO_P72);

    // Fill command buffer
    pn532_packetbuffer[0] = PN532_COMMAND_WRITEGPIO;
    pn532_packetbuffer[1] = 0x00;  // P3 Pins are not changed
    pn532_packetbuffer[2] = PN532_GPIO_VALIDATIONBIT | pinstate;  // P7 Pins

    PN532_DBG_MSG("Writing P7 GPIO: ");
    DMSG_HEX(pn532_packetbuffer[1]);
    PN532_DBG_MSG("\n");

    // Send the WRITEGPIO command (0x0E)
    if (HAL(writeCommand)(pn532_packetbuffer, 3, 0, 0))
        return 0;

    return (0 < HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000));
}

/**************************************************************************/
/*!
    Reads the state of the PN532's GPIO pins

    @returns An 8-bit value containing the pin state where:

             pinState[0]  = P30
             pinState[1]  = P31
             pinState[2]  = P32
             pinState[3]  = P33
             pinState[4]  = P34
             pinState[5]  = P35
*/
/**************************************************************************/
uint8_t PN532_readGPIO(void)
{
    pn532_packetbuffer[0] = PN532_COMMAND_READGPIO;

    // Send the READGPIO command (0x0C)
    if (HAL(writeCommand)(pn532_packetbuffer, 1, 0, 0))
        return 0x0;

    HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000);

    /* READGPIO response without prefix and suffix should be in the following format:

      byte            Description
      -------------   ------------------------------------------
      b0              P3 GPIO Pins
      b1              P7 GPIO Pins (not used ... taken by I2C)
      b2              Interface Mode Pins (not used ... bus select pins)
    */


    PN532_DBG_MSG("P3 GPIO: "); DMSG_HEX(pn532_packetbuffer[7]);
    PN532_DBG_MSG("P7 GPIO: "); DMSG_HEX(pn532_packetbuffer[8]);
    PN532_DBG_MSG("I0I1 GPIO: "); DMSG_HEX(pn532_packetbuffer[9]);
    PN532_DBG_MSG("\n");

    return pn532_packetbuffer[0];
}

/**************************************************************************/
/*!
    @brief  Configures the SAM (Secure Access Module)
*/
/**************************************************************************/
bool PN532_SAMConfig(void)
{
    pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
    pn532_packetbuffer[1] = 0x01; // normal mode;
    pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
    pn532_packetbuffer[3] = 0x01; // use IRQ pin!

    if (HAL(writeCommand)(pn532_packetbuffer, 4, 0, 0))
        return false;

    return (0 < HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000));
}

/**************************************************************************/
/*!
    Sets the MxRtyPassiveActivation uint8_t of the RFConfiguration register

    @param  maxRetries    0xFF to wait forever, 0x00..0xFE to timeout
                          after mxRetries

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
bool PN532_setPassiveActivationRetries(uint8_t maxRetries)
{
    pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
    pn532_packetbuffer[1] = 5;    // Config item 5 (MaxRetries)
    pn532_packetbuffer[2] = 0xFF; // MxRtyATR (default = 0xFF)
    pn532_packetbuffer[3] = 0x01; // MxRtyPSL (default = 0x01)
    pn532_packetbuffer[4] = maxRetries;

    if (HAL(writeCommand)(pn532_packetbuffer, 5, 0, 0))
        return 0x0;  // no ACK

    return (0 < HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000));
}

bool PN532_rfConfiguration(uint8_t item, uint8_t dataLen, const uint8_t* data)
{
    pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
    pn532_packetbuffer[1] = item;

    if (HAL(writeCommand)(pn532_packetbuffer, 2, data, dataLen))
        return 0x0;  // no ACK

    return (0 < HAL(readResponse)(pn532_packetbuffer, 0, 1000));
}

/***** ISO14443A Commands ******/

/**************************************************************************/
/*!
    Waits for an ISO14443A target to enter the field

    @param  cardBaudRate  Baud rate of the card
    @param  uid           Pointer to the array that will be populated
                          with the card's UID (up to 7 bytes)
    @param  uidLength     Pointer to the variable that will hold the
                          length of the card's UID.
    @param  timeout       The number of tries before timing out
    @param  inlist        If set to true, the card will be inlisted

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
bool PN532_readPassiveTargetID(uint8_t cardbaudrate, uint8_t *uid, uint8_t *uidLength, uint16_t timeout, bool inlist)
{
    pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1;  // max 1 cards at once (we can set this to 2 later)
    pn532_packetbuffer[2] = cardbaudrate;

    if (HAL(writeCommand)(pn532_packetbuffer, 3, 0, 0)) {
        return 0x0;  // command failed
    }

    // read data packet
    if (HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), timeout) < 0) {
        return 0x0;
    }

    // check some basic stuff
    /* ISO14443A card response should be in the following format:

      byte            Description
      -------------   ------------------------------------------
      b0              Tags Found
      b1              Tag Number (only one used in this example)
      b2..3           SENS_RES
      b4              SEL_RES
      b5              NFCID Length
      b6..NFCIDLen    NFCID
    */

    if (pn532_packetbuffer[0] != 1)
        return 0;

    uint16_t sens_res = pn532_packetbuffer[2];
    sens_res <<= 8;
    sens_res |= pn532_packetbuffer[3];

    PN532_DBG_MSG("ATQA: 0x");  DMSG_HEX(sens_res);
    PN532_DBG_MSG("SAK: 0x");  DMSG_HEX(pn532_packetbuffer[4]);
    PN532_DBG_MSG("\n");

    /* Card appears to be Mifare Classic */
    *uidLength = pn532_packetbuffer[5];

    for (uint8_t i = 0; i < pn532_packetbuffer[5]; i++) {
        uid[i] = pn532_packetbuffer[6 + i];
    }

    if (inlist) {
        inListedTag = pn532_packetbuffer[1];
    }

    return 1;
}


/***** Mifare Classic Functions ******/

/**************************************************************************/
/*!
      Indicates whether the specified block number is the first block
      in the sector (block 0 relative to the current sector)
*/
/**************************************************************************/
bool PN532_mifareclassic_IsFirstBlock (uint32_t uiBlock)
{
    // Test if we are in the small or big sectors
    if (uiBlock < 128)
        return ((uiBlock) % 4 == 0);
    else
        return ((uiBlock) % 16 == 0);
}

/**************************************************************************/
/*!
      Indicates whether the specified block number is the sector trailer
*/
/**************************************************************************/
bool PN532_mifareclassic_IsTrailerBlock (uint32_t uiBlock)
{
    // Test if we are in the small or big sectors
    if (uiBlock < 128)
        return ((uiBlock + 1) % 4 == 0);
    else
        return ((uiBlock + 1) % 16 == 0);
}

/**************************************************************************/
/*!
    Tries to authenticate a block of memory on a MIFARE card using the
    INDATAEXCHANGE command.  See section 7.3.8 of the PN532 User Manual
    for more information on sending MIFARE and other commands.

    @param  uid           Pointer to a byte array containing the card UID
    @param  uidLen        The length (in bytes) of the card's UID (Should
                          be 4 for MIFARE Classic)
    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  keyNumber     Which key type to use during authentication
                          (0 = MIFARE_CMD_AUTH_A, 1 = MIFARE_CMD_AUTH_B)
    @param  keyData       Pointer to a byte array containing the 6 bytes
                          key value

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t PN532_mifareclassic_AuthenticateBlock (uint8_t *uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t *keyData)
{
    uint8_t i;

    // Hang on to the key and uid data
    memcpy (_key, keyData, 6);
    memcpy (_uid, uid, uidLen);
    _uidLen = uidLen;

    // Prepare the authentication command //
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;   /* Data Exchange Header */
    pn532_packetbuffer[1] = 1;                              /* Max card numbers */
    pn532_packetbuffer[2] = (keyNumber) ? MIFARE_CMD_AUTH_B : MIFARE_CMD_AUTH_A;
    pn532_packetbuffer[3] = blockNumber;                    /* Block Number (1K = 0..63, 4K = 0..255 */
    memcpy (pn532_packetbuffer + 4, _key, 6);
    for (i = 0; i < _uidLen; i++) {
        pn532_packetbuffer[10 + i] = _uid[i];              /* 4 bytes card ID */
    }

    if (HAL(writeCommand)(pn532_packetbuffer, 10 + _uidLen, 0, 0))
        return 0;

    // Read the response packet
    HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000);

    // Check if the response is valid and we are authenticated???
    // for an auth success it should be bytes 5-7: 0xD5 0x41 0x00
    // Mifare auth error is technically byte 7: 0x14 but anything other and 0x00 is not good
    if (pn532_packetbuffer[0] != 0x00) {
        PN532_DBG_MSG("Authentification failed\n");
        return 0;
    }

    return 1;
}

/**************************************************************************/
/*!
    Tries to read an entire 16-bytes data block at the specified block
    address.

    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  data          Pointer to the byte array that will hold the
                          retrieved data (if any)

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t PN532_mifareclassic_ReadDataBlock (uint8_t blockNumber, uint8_t *data)
{
    PN532_DBG_MSG("Trying to read 16 bytes from block ");
    DMSG_INT(blockNumber);

    /* Prepare the command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_READ;        /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */

    /* Send the command */
    if (HAL(writeCommand)(pn532_packetbuffer, 4, 0, 0)) {
        return 0;
    }

    /* Read the response packet */
    HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000);

    /* If byte 8 isn't 0x00 we probably have an error */
    if (pn532_packetbuffer[0] != 0x00) {
        return 0;
    }

    /* Copy the 16 data bytes to the output buffer        */
    /* Block content starts at byte 9 of a valid response */
    memcpy (data, pn532_packetbuffer + 1, 16);

    return 1;
}

/**************************************************************************/
/*!
    Tries to write an entire 16-bytes data block at the specified block
    address.

    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  data          The byte array that contains the data to write.

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t PN532_mifareclassic_WriteDataBlock (uint8_t blockNumber, uint8_t *data)
{
    /* Prepare the first command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_WRITE;       /* Mifare Write command = 0xA0 */
    pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */
    memcpy (pn532_packetbuffer + 4, data, 16);        /* Data Payload */

    /* Send the command */
    if (HAL(writeCommand)(pn532_packetbuffer, 20, 0, 0)) {
        return 0;
    }

    /* Read the response packet */
    return (0 < HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000));
}

/**************************************************************************/
/*!
    Formats a Mifare Classic card to store NDEF Records

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t PN532_mifareclassic_FormatNDEF (void)
{
    uint8_t sectorbuffer1[16] = {0x14, 0x01, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
    uint8_t sectorbuffer2[16] = {0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
    uint8_t sectorbuffer3[16] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0x78, 0x77, 0x88, 0xC1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // Note 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 must be used for key A
    // for the MAD sector in NDEF records (sector 0)

    // Write block 1 and 2 to the card
    if (!(PN532_mifareclassic_WriteDataBlock (1, sectorbuffer1)))
        return 0;
    if (!(PN532_mifareclassic_WriteDataBlock (2, sectorbuffer2)))
        return 0;
    // Write key A and access rights card
    if (!(PN532_mifareclassic_WriteDataBlock (3, sectorbuffer3)))
        return 0;

    // Seems that everything was OK (?!)
    return 1;
}

/**************************************************************************/
/*!
    Writes an NDEF URI Record to the specified sector (1..15)

    Note that this function assumes that the Mifare Classic card is
    already formatted to work as an "NFC Forum Tag" and uses a MAD1
    file system.  You can use the NXP TagWriter app on Android to
    properly format cards for this.

    @param  sectorNumber  The sector that the URI record should be written
                          to (can be 1..15 for a 1K card)
    @param  uriIdentifier The uri identifier code (0 = none, 0x01 =
                          "http://www.", etc.)
    @param  url           The uri text to write (max 38 characters).

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t PN532_mifareclassic_WriteNDEFURI (uint8_t sectorNumber, uint8_t uriIdentifier, const char *url)
{
    // Figure out how long the string is
    uint8_t len = strlen(url);

    // Make sure we're within a 1K limit for the sector number
    if ((sectorNumber < 1) || (sectorNumber > 15))
        return 0;

    // Make sure the URI payload is between 1 and 38 chars
    if ((len < 1) || (len > 38))
        return 0;

    // Note 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 must be used for key A
    // in NDEF records

    // Setup the sector buffer (w/pre-formatted TLV wrapper and NDEF message)
    uint8_t sectorbuffer1[16] = {0x00, 0x00, 0x03, len + 5, 0xD1, 0x01, len + 1, 0x55, uriIdentifier, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer3[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer4[16] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7, 0x7F, 0x07, 0x88, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (len <= 6) {
        // Unlikely we'll get a url this short, but why not ...
        memcpy (sectorbuffer1 + 9, url, len);
        sectorbuffer1[len + 9] = 0xFE;
    } else if (len == 7) {
        // 0xFE needs to be wrapped around to next block
        memcpy (sectorbuffer1 + 9, url, len);
        sectorbuffer2[0] = 0xFE;
    } else if ((len > 7) || (len <= 22)) {
        // Url fits in two blocks
        memcpy (sectorbuffer1 + 9, url, 7);
        memcpy (sectorbuffer2, url + 7, len - 7);
        sectorbuffer2[len - 7] = 0xFE;
    } else if (len == 23) {
        // 0xFE needs to be wrapped around to final block
        memcpy (sectorbuffer1 + 9, url, 7);
        memcpy (sectorbuffer2, url + 7, len - 7);
        sectorbuffer3[0] = 0xFE;
    } else {
        // Url fits in three blocks
        memcpy (sectorbuffer1 + 9, url, 7);
        memcpy (sectorbuffer2, url + 7, 16);
        memcpy (sectorbuffer3, url + 23, len - 24);
        sectorbuffer3[len - 22] = 0xFE;
    }

    // Now write all three blocks back to the card
    if (!(PN532_mifareclassic_WriteDataBlock (sectorNumber * 4, sectorbuffer1)))
        return 0;
    if (!(PN532_mifareclassic_WriteDataBlock ((sectorNumber * 4) + 1, sectorbuffer2)))
        return 0;
    if (!(PN532_mifareclassic_WriteDataBlock ((sectorNumber * 4) + 2, sectorbuffer3)))
        return 0;
    if (!(PN532_mifareclassic_WriteDataBlock ((sectorNumber * 4) + 3, sectorbuffer4)))
        return 0;

    // Seems that everything was OK (?!)
    return 1;
}

/***** Mifare Ultralight Functions ******/

/**************************************************************************/
/*!
    Tries to read an entire 4-bytes page at the specified address.

    @param  page        The page number (0..63 in most cases)
    @param  buffer      Pointer to the byte array that will hold the
                        retrieved data (if any)
*/
/**************************************************************************/
uint8_t PN532_mifareultralight_ReadPage (uint8_t page, uint8_t *buffer)
{
    if (page >= 64) {
        PN532_DBG_MSG("Page value out of range\n");
        return 0;
    }

    /* Prepare the command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                   /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_READ;     /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = page;                /* Page Number (0..63 in most cases) */

    /* Send the command */
    if (HAL(writeCommand)(pn532_packetbuffer, 4, 0, 0)) {
        return 0;
    }

    /* Read the response packet */
    HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000);

    /* If byte 8 isn't 0x00 we probably have an error */
    if (pn532_packetbuffer[0] == 0x00) {
        /* Copy the 4 data bytes to the output buffer         */
        /* Block content starts at byte 9 of a valid response */
        /* Note that the command actually reads 16 bytes or 4  */
        /* pages at a time ... we simply discard the last 12  */
        /* bytes                                              */
        memcpy (buffer, pn532_packetbuffer + 1, 4);
    } else {
        return 0;
    }

    // Return OK signal
    return 1;
}

/**************************************************************************/
/*!
    Tries to write an entire 4-bytes data buffer at the specified page
    address.

    @param  page     The page number to write into.  (0..63).
    @param  buffer   The byte array that contains the data to write.

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t PN532_mifareultralight_WritePage (uint8_t page, uint8_t *buffer)
{
    /* Prepare the first command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                           /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_WRITE_ULTRALIGHT; /* Mifare UL Write cmd = 0xA2 */
    pn532_packetbuffer[3] = page;                        /* page Number (0..63) */
    memcpy (pn532_packetbuffer + 4, buffer, 4);          /* Data Payload */

    /* Send the command */
    if (HAL(writeCommand)(pn532_packetbuffer, 8, 0, 0)) {
        return 0;
    }

    /* Read the response packet */
    return (0 < HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000));
}

/**************************************************************************/
/*!
    @brief  Exchanges an APDU with the currently inlisted peer

    @param  send            Pointer to data to send
    @param  sendLength      Length of the data to send
    @param  response        Pointer to response data
    @param  responseLength  Pointer to the response data length
*/
/**************************************************************************/
bool PN532_inDataExchange(uint8_t *send, uint8_t sendLength, uint8_t *response, uint8_t *responseLength)
{
    uint8_t i;

    pn532_packetbuffer[0] = 0x40; // PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = inListedTag;

    if (HAL(writeCommand)(pn532_packetbuffer, 2, send, sendLength)) {
        return false;
    }

    int16_t status = HAL(readResponse)(response, *responseLength, 1000);
    if (status < 0) {
        return false;
    }

    if ((response[0] & 0x3f) != 0) {
        PN532_DBG_MSG("Status code indicates an error\n");
        return false;
    }

    uint8_t length = status;
    length -= 1;

    if (length > *responseLength) {
        length = *responseLength; // silent truncation...
    }

    for (uint8_t i = 0; i < length; i++) {
        response[i] = response[i + 1];
    }
    *responseLength = length;

    return true;
}

/**************************************************************************/
/*!
    @brief  'InLists' a passive target. PN532 acting as reader/initiator,
            peer acting as card/responder.
*/
/**************************************************************************/
bool PN532_inListPassiveTarget(uint8_t cardbaudrate, uint8_t initiatorDataLen, uint8_t* initiatorData, uint16_t timeout)
{
    pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1;
    pn532_packetbuffer[2] = cardbaudrate;

    PN532_DBG_MSG("inList passive target\n");

    if (HAL(writeCommand)(pn532_packetbuffer, 3, initiatorData, initiatorDataLen)) {
        return false;
    }

    int16_t status = HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), timeout);
    if (status < 0) {
        return false;
    }

    if (pn532_packetbuffer[0] != 1) {
        return false;
    }

    inListedTag = pn532_packetbuffer[1];

    return true;
}

/**************************************************************************/
/*!
    @brief  Basic data exchanges between the PN532 and a target.

    @param  send            Pointer to data to send
    @param  sendLength      Length of the data to send
    @param  response        Pointer to response data
    @param  responseLength  Pointer to the response data length
*/
/**************************************************************************/
bool PN532_inCommunicateThru(const uint8_t *send, uint8_t sendLength, uint8_t *response, uint8_t *responseLength)
{
    uint8_t i;

    pn532_packetbuffer[0] = PN532_COMMAND_INCOMMUNICATETHRU;

    if (HAL(writeCommand)(pn532_packetbuffer, 1, send, sendLength)) {
        return false;
    }

    int16_t status = HAL(readResponse)(response, *responseLength, 1000);
    if (status < 0) {
        return false;
    }

    if ((response[0] & 0x3f) != 0) {
        PN532_DBG_MSG("Status code indicates an error\n");
        return false;
    }

    uint8_t length = status;
    length -= 1;

    if (length > *responseLength) {
        length = *responseLength; // silent truncation...
    }

    for (uint8_t i = 0; i < length; i++) {
        response[i] = response[i + 1];
    }
    *responseLength = length;

    return true;
}

bool PN532_writeRegister(uint8_t number, const uint16_t *addrs, const uint8_t *values)
{
    uint8_t j = 1;
    pn532_packetbuffer[0] = PN532_COMMAND_WRITEREGISTER;
    for (uint8_t i = 0; i < number; ++i) {
        pn532_packetbuffer[j++] = addrs[i] >> 8;
        pn532_packetbuffer[j++] = addrs[i] & 0xff;
        pn532_packetbuffer[j++] = values[i];
    }
    if (HAL(writeCommand)(pn532_packetbuffer, j, 0, 0)) {
        return false;
    }
    int16_t status = HAL(readResponse)(pn532_packetbuffer, 0, 100);
    if (status < 0) {
        return false;
    }
    return true;
}

bool PN532_readRegister(uint8_t number, const uint16_t *addrs, uint8_t *values)
{
    uint8_t j = 1;
    pn532_packetbuffer[0] = PN532_COMMAND_READREGISTER;
    for (uint8_t i = 0; i < number; ++i) {
        pn532_packetbuffer[j++] = addrs[i] >> 8;
        pn532_packetbuffer[j++] = addrs[i] & 0xff;
    }
    if (HAL(writeCommand)(pn532_packetbuffer, j, 0, 0)) {
        return false;
    }
    int16_t status = HAL(readResponse)(values, number, 100);
    if (status < 0) {
        return false;
    }
    return true;
}

int8_t PN532_tgInitAsTarget(const uint8_t* command, const uint8_t len, const uint16_t timeout)
{

    int8_t status = HAL(writeCommand)(command, len, 0, 0);
    if (status < 0) {
        return -1;
    }

    status = HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), timeout);
    if (status > 0) {
        return 1;
    } else if (PN532_TIMEOUT == status) {
        return 0;
    } else {
        return -2;
    }
}

/**
 * Peer to Peer
 */
int8_t PN532_tgInitAsTargetDefault(uint16_t timeout)
{
    const uint8_t command[] = {
        PN532_COMMAND_TGINITASTARGET,
        0,
        0x00, 0x00,         //SENS_RES
        0x00, 0x00, 0x00,   //NFCID1
        0x40,               //SEL_RES

        0x01, 0xFE, 0x0F, 0xBB, 0xBA, 0xA6, 0xC9, 0x89, // POL_RES
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF,

        0x01, 0xFE, 0x0F, 0xBB, 0xBA, 0xA6, 0xC9, 0x89, 0x00, 0x00, //NFCID3t: Change this to desired value

        0x06, 0x46,  0x66, 0x6D, 0x01, 0x01, 0x10, 0x00// LLCP magic number and version parameter
    };
    return PN532_tgInitAsTarget(command, sizeof(command), timeout);
}

int16_t PN532_tgGetData(uint8_t *buf, uint8_t len)
{
    buf[0] = PN532_COMMAND_TGGETDATA;

    if (HAL(writeCommand)(buf, 1, 0, 0)) {
        return -1;
    }

    int16_t status = HAL(readResponse)(buf, len, 3000);
    if (0 >= status) {
        return status;
    }

    uint16_t length = status - 1;


    if (buf[0] != 0) {
        PN532_DBG_MSG("status is not ok\n");
        return -5;
    }

    for (uint8_t i = 0; i < length; i++) {
        buf[i] = buf[i + 1];
    }

    return length;
}

bool PN532_tgSetData(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    if (hlen > (sizeof(pn532_packetbuffer) - 1)) {
        if ((body != 0) || (header == pn532_packetbuffer)) {
            PN532_DBG_MSG("tgSetData:buffer too small\n");
            return false;
        }

        pn532_packetbuffer[0] = PN532_COMMAND_TGSETDATA;
        if (HAL(writeCommand)(pn532_packetbuffer, 1, header, hlen)) {
            return false;
        }
    } else {
        for (int8_t i = hlen - 1; i >= 0; i--) {
            pn532_packetbuffer[i + 1] = header[i];
        }
        pn532_packetbuffer[0] = PN532_COMMAND_TGSETDATA;

        if (HAL(writeCommand)(pn532_packetbuffer, hlen + 1, body, blen)) {
            return false;
        }
    }

    if (0 > HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), 3000)) {
        return false;
    }

    if (0 != pn532_packetbuffer[0]) {
        return false;
    }

    return true;
}

int16_t PN532_inRelease(const uint8_t relevantTarget)
{

    pn532_packetbuffer[0] = PN532_COMMAND_INRELEASE;
    pn532_packetbuffer[1] = relevantTarget;

    if (HAL(writeCommand)(pn532_packetbuffer, 2, 0, 0)) {
        return 0;
    }

    // read data packet
    return HAL(readResponse)(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000);
}

void PN532_configFor14443B()
{
    uint8_t param[] = {1};
    PN532_rfConfiguration(1, 1, param); //RF On
    Delay_ms(50);
    setPN532RegsForTypeB();
}

void PN532_resetConfigFor14443B()
{
    // uint8_t param[] = {0};
    restorePN532RegsForTypeB();
    // rfConfiguration(1, 1, param); //RF Off
}

bool PN532_readTsighuaStuCard(uint8_t cardId[3], uint8_t expire[3], char studentId[11])
{
    bool result;
    PN532_configFor14443B();
    result = doReadTsighuaStuCard(cardId, expire, studentId);
    return result;
}
static bool doReadTsighuaStuCard(uint8_t cardId[3], uint8_t expire[3], char studentId[11])
{
    uint8_t sz;

    static uint8_t cmd_atqb[] = {0x05, 0, 0, 0x71, 0xff};
    // ComputeCrc(CRC_B, cmd_atqb, sizeof(cmd_atqb)-2, cmd_atqb+3, cmd_atqb+4);
    sz = sizeof(pn532_packetbuffer);
    if (!PN532_inCommunicateThru(cmd_atqb, sizeof(cmd_atqb), pn532_packetbuffer, &sz))
        return false;
    PN532_DBG_MSG("ATQB returned"); DMSG_INT(sz); PN532_DBG_MSG(" bytes\n");

    uint8_t cmd_attrib[] = {
        0x1d,
        /* PUPI */ pn532_packetbuffer[1], pn532_packetbuffer[2], pn532_packetbuffer[3], pn532_packetbuffer[4],
        0x00, 0x08, 0x00, 0x00,
        /* CRC */ 0, 0
    };
    ComputeCrc(CRC_B, cmd_attrib, sizeof(cmd_attrib) - 2, cmd_attrib + 9, cmd_attrib + 10);
    sz = sizeof(pn532_packetbuffer);
    if (!PN532_inCommunicateThru(cmd_attrib, sizeof(cmd_attrib), pn532_packetbuffer, &sz))
        return false;
    PN532_DBG_MSG("ATTRIB returned"); DMSG_INT(sz); PN532_DBG_MSG(" bytes\n");

    static uint8_t cmd_cid[] = {0x0b, 0x00, 0x00, 0xb0, 0x95, 0x00, 0x0f, 0x5d, 0xd3};
    sz = sizeof(pn532_packetbuffer);
    if (!PN532_inCommunicateThru(cmd_cid, sizeof(cmd_cid), pn532_packetbuffer, &sz))
        return false;
    PN532_DBG_MSG("CMD-CID returned"); DMSG_INT(sz); PN532_DBG_MSG(" bytes\n");
    for (int i = 0; i < 3; ++i) {
        cardId[i] = pn532_packetbuffer[9 + i];
    }
    for (int i = 0; i < 3; ++i) {
        expire[i] = pn532_packetbuffer[14 + i];
    }

    static uint8_t cmd_2[] = {0x0b, 0x00, 0x00, 0xa4, 0x04, 0x00, 0x09, 0xa0, 0x00, 0x00, 0x00, 0x03, 0x86, 0x98, 0x07, 0x01, 0x6e, 0x1b};
    sz = sizeof(pn532_packetbuffer);
    if (!PN532_inCommunicateThru(cmd_2, sizeof(cmd_2), pn532_packetbuffer, &sz))
        return false;
    PN532_DBG_MSG("CMD-2 returned"); DMSG_INT(sz); PN532_DBG_MSG(" bytes\n");

    static uint8_t cmd_3[] = {0x0a, 0x00, 0x00, 0xb0, 0x95, 0x00, 0x1e, 0x80, 0x4d};
    sz = sizeof(pn532_packetbuffer);
    if (!PN532_inCommunicateThru(cmd_3, sizeof(cmd_3), pn532_packetbuffer, &sz))
        return false;
    PN532_DBG_MSG("CMD-3 returned"); DMSG_INT(sz); PN532_DBG_MSG(" bytes\n");

    static uint8_t cmd_sid[] = {0x0b, 0x00, 0x00, 0xb0, 0x96, 0x1c, 0x0a, 0xa5, 0x57};
    sz = sizeof(pn532_packetbuffer);
    if (!PN532_inCommunicateThru(cmd_sid, sizeof(cmd_sid), pn532_packetbuffer, &sz))
        return false;
    PN532_DBG_MSG("CMD-SID returned"); DMSG_INT(sz); PN532_DBG_MSG(" bytes\n");
    for (int i = 0; i < 10; ++i) {
        studentId[i] = pn532_packetbuffer[2 + i];
    }
    studentId[10] = '\0';

    return true;
}
bool PN532_stuCardIsPresent()
{
    uint8_t sz;
    static uint8_t cmd_3[] = {0x0a, 0x00, 0x00, 0xb0, 0x95, 0x00, 0x1e, 0x80, 0x4d};
    sz = sizeof(pn532_packetbuffer);
    if (!PN532_inCommunicateThru(cmd_3, sizeof(cmd_3), pn532_packetbuffer, &sz))
        return false;
    PN532_DBG_MSG("CMD-3 returned %d bytes\n", sz);
    return true;
}

static unsigned short UpdateCrc(unsigned char ch, unsigned short *lpwCrc)
{
    ch = (ch ^ (unsigned char)((*lpwCrc) & 0x00FF));
    ch = (ch ^ (ch << 4));
    *lpwCrc = (*lpwCrc >> 8) ^ ((unsigned short)ch << 8) ^ ((unsigned short)ch << 3) ^ ((unsigned short)ch >> 4);
    return (*lpwCrc);
}
static void ComputeCrc(uint8_t CRCType, uint8_t *Data, uint8_t Length, uint8_t *TransmitFirst, uint8_t *TransmitSecond)
{
    unsigned char chBlock;
    unsigned short wCrc;
    switch (CRCType) {
    case CRC_A:
        wCrc = 0x6363; /* ITU-V.41 */
        break;
    case CRC_B:
        wCrc = 0xFFFF; /* ISO/IEC 13239 (formerly ISO/IEC 3309) */
        break;
    default:
        return;
    }
    do {
        chBlock = *Data++; UpdateCrc(chBlock, &wCrc);
    } while (--Length);
    if (CRCType == CRC_B)
        wCrc = ~wCrc; /* ISO/IEC 13239 (formerly ISO/IEC 3309) */ *TransmitFirst = (uint8_t) (wCrc & 0xFF);
    *TransmitSecond = (uint8_t) ((wCrc >> 8) & 0xFF);
    return;
}

static void setPN532RegsForTypeB()
{
    if (!PN532_readRegister(6, registerAddrForTypeB, registerValueBackup)) {
        PN532_DBG_MSG("failed to backup registers for type B\n");
        return;
    }
    if (!PN532_readRegister(6, registerAddrForTypeB + 6, registerValueBackup + 6)) {
        PN532_DBG_MSG("failed to backup registers for type B\n");
        return;
    }
    if (!PN532_writeRegister(6, registerAddrForTypeB, registerValueForTypeB)) {
        PN532_DBG_MSG("failed to set registers for type B\n");
        return;
    }
    if (!PN532_writeRegister(6, registerAddrForTypeB + 6, registerValueForTypeB + 6)) {
        PN532_DBG_MSG("failed to set registers for type B\n");
        return;
    }
}

static void restorePN532RegsForTypeB()
{
    if (!PN532_writeRegister(6, registerAddrForTypeB, registerValueBackup)) {
        PN532_DBG_MSG("failed to restore registers for type B\n");
        return;
    }
    if (!PN532_writeRegister(6, registerAddrForTypeB + 6, registerValueBackup + 6)) {
        PN532_DBG_MSG("failed to restore registers for type B\n");
        return;
    }
}

uint8_t *PN532_getBuffer(uint8_t *len)
{
    *len = sizeof(pn532_packetbuffer) - 4;
    return pn532_packetbuffer;
};
