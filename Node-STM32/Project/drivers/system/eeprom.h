#ifndef EEPROM_H__
#define EEPROM_H__


void FlashEEP_WriteHalfWords(uint16_t* data, uint32_t length, uint32_t addr);

#endif