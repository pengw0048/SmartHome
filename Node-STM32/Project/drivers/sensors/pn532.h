/**************************************************************************/
/*!
    @file     PN532.h
    @author   Adafruit Industries & Seeed Studio
    @license  BSD
*/
/**************************************************************************/

#ifndef __PN532_H__
#define __PN532_H__

#include "pn532Interface.h"
#include "common.h"

// PN532 Commands
#define PN532_COMMAND_DIAGNOSE              (0x00)
#define PN532_COMMAND_GETFIRMWAREVERSION    (0x02)
#define PN532_COMMAND_GETGENERALSTATUS      (0x04)
#define PN532_COMMAND_READREGISTER          (0x06)
#define PN532_COMMAND_WRITEREGISTER         (0x08)
#define PN532_COMMAND_READGPIO              (0x0C)
#define PN532_COMMAND_WRITEGPIO             (0x0E)
#define PN532_COMMAND_SETSERIALBAUDRATE     (0x10)
#define PN532_COMMAND_SETPARAMETERS         (0x12)
#define PN532_COMMAND_SAMCONFIGURATION      (0x14)
#define PN532_COMMAND_POWERDOWN             (0x16)
#define PN532_COMMAND_RFCONFIGURATION       (0x32)
#define PN532_COMMAND_RFREGULATIONTEST      (0x58)
#define PN532_COMMAND_INJUMPFORDEP          (0x56)
#define PN532_COMMAND_INJUMPFORPSL          (0x46)
#define PN532_COMMAND_INLISTPASSIVETARGET   (0x4A)
#define PN532_COMMAND_INATR                 (0x50)
#define PN532_COMMAND_INPSL                 (0x4E)
#define PN532_COMMAND_INDATAEXCHANGE        (0x40)
#define PN532_COMMAND_INCOMMUNICATETHRU     (0x42)
#define PN532_COMMAND_INDESELECT            (0x44)
#define PN532_COMMAND_INRELEASE             (0x52)
#define PN532_COMMAND_INSELECT              (0x54)
#define PN532_COMMAND_INAUTOPOLL            (0x60)
#define PN532_COMMAND_TGINITASTARGET        (0x8C)
#define PN532_COMMAND_TGSETGENERALBYTES     (0x92)
#define PN532_COMMAND_TGGETDATA             (0x86)
#define PN532_COMMAND_TGSETDATA             (0x8E)
#define PN532_COMMAND_TGSETMETADATA         (0x94)
#define PN532_COMMAND_TGGETINITIATORCOMMAND (0x88)
#define PN532_COMMAND_TGRESPONSETOINITIATOR (0x90)
#define PN532_COMMAND_TGGETTARGETSTATUS     (0x8A)

#define PN532_RESPONSE_INDATAEXCHANGE       (0x41)
#define PN532_RESPONSE_INLISTPASSIVETARGET  (0x4B)


#define PN532_MIFARE_ISO14443A              (0x00)
#define PN532_106KBPS_ISO14443B             (0x03)

// Mifare Commands
#define MIFARE_CMD_AUTH_A                   (0x60)
#define MIFARE_CMD_AUTH_B                   (0x61)
#define MIFARE_CMD_READ                     (0x30)
#define MIFARE_CMD_WRITE                    (0xA0)
#define MIFARE_CMD_WRITE_ULTRALIGHT         (0xA2)
#define MIFARE_CMD_TRANSFER                 (0xB0)
#define MIFARE_CMD_DECREMENT                (0xC0)
#define MIFARE_CMD_INCREMENT                (0xC1)
#define MIFARE_CMD_STORE                    (0xC2)

// Prefixes for NDEF Records (to identify record type)
#define NDEF_URIPREFIX_NONE                 (0x00)
#define NDEF_URIPREFIX_HTTP_WWWDOT          (0x01)
#define NDEF_URIPREFIX_HTTPS_WWWDOT         (0x02)
#define NDEF_URIPREFIX_HTTP                 (0x03)
#define NDEF_URIPREFIX_HTTPS                (0x04)
#define NDEF_URIPREFIX_TEL                  (0x05)
#define NDEF_URIPREFIX_MAILTO               (0x06)
#define NDEF_URIPREFIX_FTP_ANONAT           (0x07)
#define NDEF_URIPREFIX_FTP_FTPDOT           (0x08)
#define NDEF_URIPREFIX_FTPS                 (0x09)
#define NDEF_URIPREFIX_SFTP                 (0x0A)
#define NDEF_URIPREFIX_SMB                  (0x0B)
#define NDEF_URIPREFIX_NFS                  (0x0C)
#define NDEF_URIPREFIX_FTP                  (0x0D)
#define NDEF_URIPREFIX_DAV                  (0x0E)
#define NDEF_URIPREFIX_NEWS                 (0x0F)
#define NDEF_URIPREFIX_TELNET               (0x10)
#define NDEF_URIPREFIX_IMAP                 (0x11)
#define NDEF_URIPREFIX_RTSP                 (0x12)
#define NDEF_URIPREFIX_URN                  (0x13)
#define NDEF_URIPREFIX_POP                  (0x14)
#define NDEF_URIPREFIX_SIP                  (0x15)
#define NDEF_URIPREFIX_SIPS                 (0x16)
#define NDEF_URIPREFIX_TFTP                 (0x17)
#define NDEF_URIPREFIX_BTSPP                (0x18)
#define NDEF_URIPREFIX_BTL2CAP              (0x19)
#define NDEF_URIPREFIX_BTGOEP               (0x1A)
#define NDEF_URIPREFIX_TCPOBEX              (0x1B)
#define NDEF_URIPREFIX_IRDAOBEX             (0x1C)
#define NDEF_URIPREFIX_FILE                 (0x1D)
#define NDEF_URIPREFIX_URN_EPC_ID           (0x1E)
#define NDEF_URIPREFIX_URN_EPC_TAG          (0x1F)
#define NDEF_URIPREFIX_URN_EPC_PAT          (0x20)
#define NDEF_URIPREFIX_URN_EPC_RAW          (0x21)
#define NDEF_URIPREFIX_URN_EPC              (0x22)
#define NDEF_URIPREFIX_URN_NFC              (0x23)

#define PN532_GPIO_VALIDATIONBIT            (0x80)
#define PN532_GPIO_P30                      (0)
#define PN532_GPIO_P31                      (1)
#define PN532_GPIO_P32                      (2)
#define PN532_GPIO_P33                      (3)
#define PN532_GPIO_P34                      (4)
#define PN532_GPIO_P35                      (5)

#define PN532_GPIO_P71                      (1)
#define PN532_GPIO_P72                      (2)

/**************** from pn53x-internal.h **********/
// Register addresses
#define PN53X_REG_Control_switch_rng 0x6106
#define PN53X_REG_CIU_Mode 0x6301
#define PN53X_REG_CIU_TxMode 0x6302
#define PN53X_REG_CIU_RxMode 0x6303
#define PN53X_REG_CIU_TxControl 0x6304
#define PN53X_REG_CIU_TxAuto 0x6305
#define PN53X_REG_CIU_TxSel 0x6306
#define PN53X_REG_CIU_RxSel 0x6307
#define PN53X_REG_CIU_RxThreshold 0x6308
#define PN53X_REG_CIU_Demod 0x6309
#define PN53X_REG_CIU_FelNFC1 0x630A
#define PN53X_REG_CIU_FelNFC2 0x630B
#define PN53X_REG_CIU_MifNFC 0x630C
#define PN53X_REG_CIU_ManualRCV 0x630D
#define PN53X_REG_CIU_TypeB 0x630E
// #define PN53X_REG_- 0x630F
// #define PN53X_REG_- 0x6310
#define PN53X_REG_CIU_CRCResultMSB 0x6311
#define PN53X_REG_CIU_CRCResultLSB 0x6312
#define PN53X_REG_CIU_GsNOFF 0x6313
#define PN53X_REG_CIU_ModWidth 0x6314
#define PN53X_REG_CIU_TxBitPhase 0x6315
#define PN53X_REG_CIU_RFCfg 0x6316
#define PN53X_REG_CIU_GsNOn 0x6317
#define PN53X_REG_CIU_CWGsP 0x6318
#define PN53X_REG_CIU_ModGsP 0x6319
#define PN53X_REG_CIU_TMode 0x631A
#define PN53X_REG_CIU_TPrescaler 0x631B
#define PN53X_REG_CIU_TReloadVal_hi 0x631C
#define PN53X_REG_CIU_TReloadVal_lo 0x631D
#define PN53X_REG_CIU_TCounterVal_hi 0x631E
#define PN53X_REG_CIU_TCounterVal_lo 0x631F
// #define PN53X_REG_- 0x6320
#define PN53X_REG_CIU_TestSel1 0x6321
#define PN53X_REG_CIU_TestSel2 0x6322
#define PN53X_REG_CIU_TestPinEn 0x6323
#define PN53X_REG_CIU_TestPinValue 0x6324
#define PN53X_REG_CIU_TestBus 0x6325
#define PN53X_REG_CIU_AutoTest 0x6326
#define PN53X_REG_CIU_Version 0x6327
#define PN53X_REG_CIU_AnalogTest 0x6328
#define PN53X_REG_CIU_TestDAC1 0x6329
#define PN53X_REG_CIU_TestDAC2 0x632A
#define PN53X_REG_CIU_TestADC 0x632B
// #define PN53X_REG_- 0x632C
// #define PN53X_REG_- 0x632D
// #define PN53X_REG_- 0x632E
#define PN53X_REG_CIU_RFlevelDet 0x632F
#define PN53X_REG_CIU_SIC_CLK_en 0x6330
#define PN53X_REG_CIU_Command 0x6331
#define PN53X_REG_CIU_CommIEn 0x6332
#define PN53X_REG_CIU_DivIEn 0x6333
#define PN53X_REG_CIU_CommIrq 0x6334
#define PN53X_REG_CIU_DivIrq 0x6335
#define PN53X_REG_CIU_Error 0x6336
#define PN53X_REG_CIU_Status1 0x6337
#define PN53X_REG_CIU_Status2 0x6338
#define PN53X_REG_CIU_FIFOData 0x6339
#define PN53X_REG_CIU_FIFOLevel 0x633A
#define PN53X_REG_CIU_WaterLevel 0x633B
#define PN53X_REG_CIU_Control 0x633C
#define PN53X_REG_CIU_BitFraming 0x633D
#define PN53X_REG_CIU_Coll 0x633E
/**************** end pn53x-internal.h **********/

void PN532_begin(void);

// Generic PN532 functions
bool PN532_SAMConfig(void);
uint32_t PN532_getFirmwareVersion(void);
bool PN532_writeGPIO(uint8_t pinstate);
bool PN532_writeGPIOP7(uint8_t pinstate);
uint8_t PN532_readGPIO(void);
bool PN532_setPassiveActivationRetries(uint8_t maxRetries);

/**
* @brief    Init PN532 as a target
* @param    timeout max time to wait, 0 means no timeout
* @return   > 0     success
*           = 0     timeout
*           < 0     failed
*/
int8_t PN532_tgInitAsTargetDefault(uint16_t timeout);
int8_t PN532_tgInitAsTarget(const uint8_t* command, const uint8_t len, const uint16_t timeout);

int16_t PN532_tgGetData(uint8_t *buf, uint8_t len);
bool PN532_tgSetData(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen);

int16_t PN532_inRelease(const uint8_t relevantTarget);
bool PN532_inListPassiveTarget(uint8_t cardbaudrate, uint8_t initiatorDataLen, uint8_t* initiatorData, uint16_t timeout);
bool PN532_inCommunicateThru(const uint8_t *send, uint8_t sendLength, uint8_t *response, uint8_t *responseLength);
bool PN532_writeRegister(uint8_t number, const uint16_t *addrs, const uint8_t *values);
bool PN532_readRegister(uint8_t number, const uint16_t *addrs, uint8_t *values);
bool PN532_rfConfiguration(uint8_t item, uint8_t dataLen, const uint8_t* data);

void PN532_configFor14443B(void);
bool PN532_readTsighuaStuCard(uint8_t cardId[3], uint8_t expire[3], char studentId[11]);
bool PN532_stuCardIsPresent(void);
void PN532_resetConfigFor14443B(void);

// ISO14443A functions
bool PN532_readPassiveTargetID(uint8_t cardbaudrate, uint8_t *uid, uint8_t *uidLength, uint16_t timeout, bool inlist);
bool PN532_inDataExchange(uint8_t *send, uint8_t sendLength, uint8_t *response, uint8_t *responseLength);

// Mifare Classic functions
bool PN532_mifareclassic_IsFirstBlock (uint32_t uiBlock);
bool PN532_mifareclassic_IsTrailerBlock (uint32_t uiBlock);
uint8_t PN532_mifareclassic_AuthenticateBlock (uint8_t *uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t *keyData);
uint8_t PN532_mifareclassic_ReadDataBlock (uint8_t blockNumber, uint8_t *data);
uint8_t PN532_mifareclassic_WriteDataBlock (uint8_t blockNumber, uint8_t *data);
uint8_t PN532_mifareclassic_FormatNDEF (void);
uint8_t PN532_mifareclassic_WriteNDEFURI (uint8_t sectorNumber, uint8_t uriIdentifier, const char *url);

// Mifare Ultralight functions
uint8_t PN532_mifareultralight_ReadPage (uint8_t page, uint8_t *buffer);
uint8_t PN532_mifareultralight_WritePage (uint8_t page, uint8_t *buffer);

// Help functions to display formatted text
void PN532_PrintHex(const uint8_t *data, const uint32_t numBytes);
void PN532_PrintHexChar(const uint8_t *pbtData, const uint32_t numBytes);

uint8_t *PN532_getBuffer(uint8_t *len);

#endif
