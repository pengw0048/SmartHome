#include "common.h"
#include "eeprom.h"
#include <string.h>

#if defined(STM32F10X_HD)
#define PAGE_SIZE 2048
#elif defined(STM32F10X_MD) || defined(STM32F10X_LD)
#define PAGE_SIZE 1024
#else
#error Unknown flash page size
#endif

static uint8_t pageBuf[PAGE_SIZE];

static void programPage(uint32_t addr, uint8_t* data)
{
    for (uint32_t i = 0; i < PAGE_SIZE; i+=2){
        FLASH_ProgramHalfWord(addr+i, *(uint16_t*)(data+i));
    }
    // for (uint32_t i = 0; i < 20; i+=2)
    //     printf("%08x=%lu %lu\r\n", addr+i, *(uint16_t*)(data+i), *(uint16_t*)(addr+i));
}

void FlashEEP_WriteHalfWords(uint16_t* data, uint32_t length, uint32_t addr)
{
    uint32_t page, off, page_start;
    page = (addr - FLASH_BASE)/PAGE_SIZE;
    page_start = page*PAGE_SIZE + FLASH_BASE;
    off = addr - page_start;
    DBG_MSG("Write Flash Addr %08x @ Page %lu Off=%lu", addr, page, off);
    RCC_HSICmd(ENABLE);
    FLASH_Unlock();
    while(length > 0){
        DBG_MSG("page_start=%08x off=%lu size=%lu", page_start, off, length*sizeof(uint16_t));
        if(off>0 || length*sizeof(uint16_t)<PAGE_SIZE)
            memcpy(pageBuf, (void*)page_start, PAGE_SIZE);
        FLASH_ErasePage(page_start);
        while(off < PAGE_SIZE && length > 0){
            *(uint16_t*)(pageBuf+off) = *(data++);
            off += 2;
            length--;
        }
        programPage(page_start, pageBuf);
        off = 0;
        page_start += PAGE_SIZE;
    }
    FLASH_Lock();
}

