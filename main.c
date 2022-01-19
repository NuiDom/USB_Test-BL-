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
void writeRowFlash(int upgradeRow);
void erasePage(void);
void readFlash(int currentWord);

bool upgrade_mode = false;
bool in_bootloader = false;
static uint32_t *upgradeBuffer[192];
//Used for USB
char message_buffer[64];
bool message_received = false;
static uint8_t readBuffer[64];
static uint8_t writeBuffer[64];
char usbCmd[10] = "";
static int upgradeRowReceived = 0;
static int upgradeBufferCounter = 0;
int currentWordCounter = 0;
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
        for(y=0; y<64; y++)
            readBuffer[y]='\0';
//        in_bootloader = true;
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
        asm("GOTO 0x2600");
    }
    return 1;
}

void clearUsbBuffers(void){
    int x=0;
    
    for(x=0; x<10; x++)
        usbCmd[x]='\0';
    
    for(x=0; x<64; x++)
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
    int t=0;
    uint8_t numBytesRead;

    numBytesRead = getsUSBUSART(readBuffer, sizeof(readBuffer));
    sscanf(readBuffer, "%s", usbCmd);
    
    for(t=0; t<numBytesRead; t++){
        if(readBuffer[t] != '\0')
        {
            message_received = true;
        }
    }
    t = 0;
    if(message_received == false)
        return;
    
    else{
        
//        if(in_bootloader == true){
//            putUSBUSART((uint8_t *)"bl\r\n",2);
//            in_bootloader = false;
//            message_received = false;
//            clearUsbBuffers();            
//        }
         
        if(numBytesRead == 64){
            for(i=0+upgradeBufferCounter; i<numBytesRead+upgradeBufferCounter; i++){
            upgradeBuffer[i] = readBuffer[t];
            t++;
            }
            upgradeBufferCounter += 64;
            message_received = false;
            clearUsbBuffers();
            
            if(upgradeBufferCounter == 192){
                //write row to flash
                writeRowFlash(upgradeRowReceived);
                upgradeRowReceived += 1;
                upgradeBufferCounter = 0;
                delay_ms(1);
//                putUSBUSART((uint8_t *)"NextRow\r\n",7);
            }
            
            else{
                putUSBUSART((uint8_t *)"64\r\n",2);
                message_received = false;
                clearUsbBuffers();
            }     
        }
    
        if((strcmp(usbCmd,"READ_MEM")==0)){
            uint32_t address = 0x2400 + (0x0002*currentWordCounter);
            uint32_t word = FLASH_ReadWord24(address);
            uint8_t word8bit[4];
            word8bit[0] = (word & 0xff000000UL) >> 24;
            word8bit[1] = (word & 0x00ff0000UL) >> 16; 
            word8bit[2] = (word & 0x0000ff00UL) >>  8;
            word8bit[3] = (word & 0x000000ffUL)      ;
            currentWordCounter += 1;
            if( USBUSARTIsTxTrfReady() == false)
                CDCTxService();
            putUSBUSART(word8bit, 4);
            
        }
        
    if(upgrade_mode == false)
    {
        if(strcmp(usbCmd,"ERASE")==0){
            erasePage();
        }
        if(strcmp(usbCmd,"JUMP")==0){
            currentWordCounter = 0;
            disableInterrupts();
            delay_ms(50);
            USBDeviceDetach();
            delay_ms(700);
            asm("GOTO 0x2600");
        }
            
        else if(strcmp(usbCmd,"UPGRADE")==0){
            message_received = false;
            clearUsbBuffers();
            putUSBUSART((uint8_t *)"NextRow\r\n",7);
        }
        else{
            message_received = false;
            clearUsbBuffers();
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
        
        else if(numBytesRead == 192){
            FLASH_Unlock(FLASH_UNLOCK_KEY);
            FLASH_WriteRow24(PROGRAM_START_ADDRESS+(0x80*upgradeRowReceived), upgradeBuffer);
            FLASH_Lock();
            putUSBUSART((uint8_t *)"OK",2);
            message_received = false;
            upgradeRowReceived++;
            numBytesRead = 0;
        }
        
        else if(strcmp(usbCmd,"JUMP")==0){
            disableInterrupts();
            delay_ms(50);
            USBDeviceDetach();
            delay_ms(700);
            asm("GOTO 0x2600");
        }
        
        else{
            message_received = false;
            clearUsbBuffers();
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
//            asm("GOTO 0x2600");
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

void writeRowFlash(int upgradeRow)
{
    FLASH_Unlock(FLASH_UNLOCK_KEY);
//    FLASH_ErasePage(0x002400);
    FLASH_WriteRow24(PROGRAM_START_ADDRESS+(0x80*upgradeRow), upgradeBuffer);
    FLASH_Lock();
}
void erasePage(void)
{
    FLASH_Unlock(FLASH_UNLOCK_KEY);
    FLASH_ErasePage(0x002400);
    FLASH_Lock();
}

void readFlash(int currentWord)
{
    FLASH_ReadWord24(0x002400 + (0x04*currentWord));
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

