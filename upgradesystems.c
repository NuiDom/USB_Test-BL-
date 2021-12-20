#include "upgradesystems.h"
#include "mcc_generated_files/memory/flash.h"
#include "mcc_generated_files/system.h"

void eraseFlash(uint32_t address);
void writeFlash(uint32_t address, uint32_t *data);

extern upgradeCmds usb;
static uint32_t upgradeData[64];

void interpretCmds(void)
{
    if(usb == erase)
    {
        eraseFlash(PROGRAM_START_ADDRESS);
        //usbWrite(DONE)
        //eraseFlash(PROGRAM_START_ADDRESS+FLASH_PAGE_SIZE);
        //usbWrite(DONE)
    }
    
    else if(usb == write)
    {
        writeFlash(PROGRAM_START_ADDRESS, upgradeData);
        //usbWrite(done)
        //writeFlash(PROGRAM_START_ADDRESS+FLASH_ROW_SIZE, upgradeData);
    }
    
    else if(usb == read)
        ;
    
    else if(usb == alive)
        ;
}

void eraseFlash(uint32_t address)
{
    FLASH_Unlock(FLASH_UNLOCK_KEY);
    FLASH_ErasePage(address);
    FLASH_Lock();    
}

void writeFlash(uint32_t address, uint32_t *data)
{
    FLASH_Unlock(FLASH_UNLOCK_KEY);
    FLASH_WriteRow24(address, data);
    FLASH_Lock();      
}
