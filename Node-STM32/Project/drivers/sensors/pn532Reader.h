#ifndef PN532_READER_H__
#define PN532_READER_H__

#include "common.h"

enum{Card_None, Card_14443A, Card_14443B};

bool PN532Reader_Init(void);
void PN532Reader_Poll(void);
bool PN532Reader_CardDetected(char** result);

#endif
