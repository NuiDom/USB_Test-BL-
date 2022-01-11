/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.170.0
        Device            :  PIC24FJ64GB202
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.61
        MPLAB 	          :  MPLAB X v5.45
*/
//BOOTLOADER
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/usb/usb.h"
#include "timer.h"
#include "mcc_generated_files/memory/flash.h"
#include <stdlib.h>

#define PROGRAM_START_ADDRESS   0x002400
#define FLASH_PAGE_SIZE         0x000400
#define FLASH_ROW_SIZE          0x000080

void DoUSBComms(void);
void disableInterrupts(void);
void erase(void);
void clearUsbBuffers(void);

bool upgrade_mode = false;
static uint32_t *upgradeBuffer[192];
//Used for USB
char message_buffer[192];
bool message_received = false;
static uint8_t readBuffer[192];
static uint8_t writeBuffer[64];
char usbCmd[10] = "";
static int upgradeRow = 0;
/*
                         Main application
 */
int main(void)
{
    // initialize the device
    if(RCON == 0){
        INTCON2bits.ALTIVT = 1;
        SYSTEM_Initialize();
        TMR1_Init();
        delay_ms(1000);
        USBDeviceInit();
        USBDeviceAttach();
        delay_ms(1000);
        USBDeviceTasks();
        delay_ms(1000);
        int y=0;
        for(y=0; y<192; y++)
            readBuffer[y]='\0';
        
        while (1)
        {
        // Add your application code
        DoUSBComms();
//        FLASH_Unlock(FLASH_UNLOCK_KEY);
//        FLASH_ErasePage(0x002400);
//        FLASH_Lock();
//        FLASH_ErasePage(0x2800);
//        FLASH_ErasePage(0x3200);
//        FLASH_ErasePage(0x3600);
//        FLASH_ErasePage(0x4000);
//        FLASH_ErasePage(0x4400);
//        FLASH_ErasePage(0x4800);
        }
    }
    else if(RCONbits.POR == 1){
        asm("GOTO 0x2400");
    }
    return 1;
}

void clearUsbBuffers(void){
    int x=0;
    
    for(x=0; x<10; x++)
        usbCmd[x]='\0';
    
    for(x=0; x<192; x++)
        readBuffer[x]='\0';
}
void DoUSBComms(void)
{
    if( USBGetDeviceState() < DEFAULT_STATE )
    {
        return;
    }

    if( USBIsDeviceSuspended()== true )
    {
        return;
    }
    uint8_t i;
    uint8_t numBytesRead;

    numBytesRead = getsUSBUSART(readBuffer, sizeof(readBuffer));
    sscanf(readBuffer, "%s", usbCmd);
    
    for(i=0; i<numBytesRead; i++){
        upgradeBuffer[i] = readBuffer[i];
        if(upgradeBuffer[i] != '\0')
        {
            message_received = true;
        }
    }
    if(message_received == false)
        return;
    
    else{
    
    if(upgrade_mode == false)
    {     
        if(strcmp(usbCmd,"JUMP")==0){
            disableInterrupts();
            delay_ms(50);
            USBDeviceDetach();
            delay_ms(700);
            asm("GOTO 0x2400");
        }
            
        else if(strcmp(usbCmd,"UPGRADE")==0){
            upgrade_mode = true;
            message_received = false;
            clearUsbBuffers();
//            putUSBUSART((uint8_t *)"OK",2);
        }
        
    }
    
    if(upgrade_mode == true)
    {
        if(strcmp(usbCmd,"1")==0){
            FLASH_Unlock(FLASH_UNLOCK_KEY);
            FLASH_ErasePage(0x002400);
            FLASH_Lock();
            message_received = false;
            clearUsbBuffers();
            putUSBUSART((uint8_t *)"OK",2);
        }
        
        if(numBytesRead == 192){
            FLASH_Unlock(FLASH_UNLOCK_KEY);
            FLASH_WriteRow24(PROGRAM_START_ADDRESS+(0x80*upgradeRow), upgradeBuffer);
            FLASH_Lock();
            putUSBUSART((uint8_t *)"OK",2);
            message_received = false;
            upgradeRow++;
            numBytesRead = 0;
        }
        
        if(strcmp(usbCmd,"JUMP")==0){
            disableInterrupts();
            delay_ms(50);
            USBDeviceDetach();
            delay_ms(700);
            asm("GOTO 0x2400");
        }
    }
    if( USBUSARTIsTxTrfReady() == true)
    {
//        uint8_t i;
//        uint8_t numBytesRead;
//
//        numBytesRead = getsUSBUSART(readBuffer, sizeof(readBuffer));
//        
//        sscanf(readBuffer, "%s", usbCmd);
//         
//        if(strcmp(usbCmd,"JUMP")==0){
//            disableInterrupts();
//            delay_ms(50);
//            USBDeviceDetach();
//            delay_ms(700);
//            asm("GOTO 0x2400");
//        }
//            
//        else if(strcmp(usbCmd,"1")==0){
//            FLASH_Unlock(FLASH_UNLOCK_KEY);
//            FLASH_ErasePage(0x002400);
//            FLASH_Lock();              
//        }
//        
//        else if(strcmp(usbCmd,"UPGRADE")==0){
//            delay_ms(50);
//            putUSBUSART((uint8_t *)"OK",2);
////            usbCmd = 0x00;
////            upgrade_mode = true;
//        }
//        
//        if(numBytesRead == 192){
//            for(i=0; i<numBytesRead; i++){
//            upgradeBuffer[i] = readBuffer[i];
//            }
//            FLASH_Unlock(FLASH_UNLOCK_KEY);
//            FLASH_WriteRow24(PROGRAM_START_ADDRESS, upgradeBuffer);
//            FLASH_Lock();
//        }
//        for(i=0; i<numBytesRead; i++)
//        {
//            message_buffer[i] = readBuffer[i];
//                if(message_buffer[i] == '.')
//                {
//                    message_received = true;
//                    putUSBUSART((uint8_t *)"Received full stop.\r\n",19);
//                    return;
//                }
//            
//            switch(readBuffer[i])
//            {
//                /* echo line feeds and returns without modification. */
//                case 0x0A:
//                case 0x0D:
//                    writeBuffer[i] = readBuffer[i];
//                    break;
//
//                /* all other characters get +1 (e.g. 'a' -> 'b') */
//                default:
//                    writeBuffer[i] = readBuffer[i] + 1;
//                    break;
//            }
//        }
//
//        if(numBytesRead > 0)
//        {
//            putUSBUSART(writeBuffer,numBytesRead);
//        }
    }
    }

    CDCTxService();
}

//void erase(void)
//{
//    // C example using MPLAB C30
//    unsigned long progAddr = 0x002400;      // Address of row to write
//    unsigned int offset;
//    //Set up pointer to the first memory location to be written
//    TBLPAG = progAddr>>16;                  // Initialize PM Page Boundary SFR
//    offset = progAddr & 0xFFFF;             // Initialize lower word of address
//    __builtin_tblwtl(offset, 0x0000);       // Set base address of erase block
//                                            // with dummy latch write
//    NVMCON = 0x4042;                        // Initialize NVMCON
//    asm("DISI #5");                         // Block all interrupts with priority <7
//                                            // for next 5 instructions
//    NVMCONbits.WR = 1;                      // check function to perform unlock
//    asm ("NOP");                            // sequence and set WR
//    asm ("NOP");
//    NVMCONbits.WREN = 0;
//}
void disableInterrupts(void)
{
    IEC0 = 0x0000;
    IEC1 = 0x0000;
    IEC2 = 0x0000;
    IEC3 = 0x0000;
    IEC4 = 0x0000;
    IEC5 = 0x0000;
    IEC6 = 0x0000;
    IEC7 = 0x0000;
}
/**
 End of File
*/

