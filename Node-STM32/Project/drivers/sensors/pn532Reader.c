#include "pn532.h"
#include "pn532Reader.h"
#include <string.h>

static uint8_t card;
static bool found;

static uint8_t uid[8];    // Buffer to store the returned UID
static uint8_t uidLength; // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

static char readableCardID[32];

static void PN532Reader_SetLedOn(bool on)
{
    if (on)
        PN532_writeGPIOP7(0);
    else
        PN532_writeGPIOP7(1 << PN532_GPIO_P71);
}

bool PN532Reader_Init(void)
{
    PN532_begin();

    uint32_t versiondata = PN532_getFirmwareVersion();
    if (!versiondata) {
        ERR_MSG("PN53x not found!");
        return false;
    }

    PN532_setPassiveActivationRetries(0xFF);
    PN532_SAMConfig();

    DBG_MSG("Found chip PN5%x with firmware ver. %d.%d",
            (versiondata >> 24) & 0xFF, (versiondata >> 16) & 0xFF, (versiondata >> 8) & 0xFF);

    return true;
}

void PN532Reader_Poll(void)
{
    bool success;

    if (found) { //A card had been found previously
        return;
    }

    if (card == Card_14443B) {
        if (PN532_stuCardIsPresent()) { //Card doesn't leave yet
            return;
        } else {
            PN532_resetConfigFor14443B();
            card = Card_None;
            PN532Reader_SetLedOn(false);
        }
    }

    // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
    // 'uid' will be populated with the UID, and uidLength will indicate
    // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
    success = PN532_readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, 100, false);
    if (card == Card_14443A) {
        if (success) {
            //Card doesn't leave yet since last detected
            PN532_inRelease(0); //Release all cards
            return;
        } else {
            card = Card_None;
            PN532Reader_SetLedOn(false);
        }
    }

    if (success) {
        DBG_MSG("Found Mifare card!");
        // Serial.print("UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
        // Serial.print("UID Value: ");
        // for (uint8_t i = 0; i < uidLength; i++) {
        //     Serial.print(" 0x"); Serial.print(uid[i], HEX);
        // }
        // Serial.println("");
        uint8_t ptr = 0;
        for (uint8_t i = 0; i < uidLength; ++i)
        {
            char byte[3];
            sprintf(byte, "%02x", uid[i]);
            readableCardID[ptr++]=byte[0];
            readableCardID[ptr++]=byte[1];
        }
        readableCardID[ptr] = '\0';

        PN532_inRelease(0); //Release all cards
        card = Card_14443A;
        found = true;
        PN532Reader_SetLedOn(true);
    } else {
        // PN532 probably timed out waiting for a card
        //Serial.println("Timed out waiting for a card");
    }

    static uint8_t AFI[] = {0};
    success = PN532_inListPassiveTarget(PN532_106KBPS_ISO14443B, sizeof(AFI) , AFI, 100);
    if (success) {
        uint8_t cardId[3];
        uint8_t expire[3];
        char studentId[11];

        PN532_inRelease(0);
        success = PN532_readTsighuaStuCard(cardId, expire, studentId);
        if (success) {
            DBG_MSG("Found student card! SN: %s", studentId);
            strcpy(readableCardID, studentId);
            card = Card_14443B;
            found = true;
            PN532Reader_SetLedOn(true);
        } else {
            PN532_resetConfigFor14443B();
        }
    } else {

    }

}

bool PN532Reader_CardDetected(char** result)
{
    if(found){
        *result = readableCardID;
        found = false;
        return true;
    }
    return false;
}
