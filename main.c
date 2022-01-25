//BOOTLOADER MAIN
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/usb/usb.h"
#include "timer.h"
#include "mcc_generated_files/memory/flash.h"
#include <stdlib.h>

#define PROGRAM_START_ADDRESS   0x00002400
#define FLASH_PAGE_SIZE         0x00000400
#define FLASH_ROW_SIZE          0x00000080
#define FLASH_WORD_SIZE         0x00000002

void DoUSBComms(void);
void disableInterrupts(void);
void clearUsbBuffers(void);

//Used for USB
static uint8_t readBuffer[64];
static uint8_t writeBuffer[64];
static uint32_t upgradeWord;
char usbCmd[10] = "";
//static int upgradeBufferCounter = 0;
uint32_t currentWordReadCounter = 0;
uint32_t currentWordWriteCounter = 0;
/*
                         Main application
 */
int main(void)
{
    // initialize the device
    if(RCON == 0){                                  //RCON reg is reset before jumping to bootloader
        INTCON2bits.ALTIVT = 1;                     //Initalizes the use of the alternate interrupt vector table
        SYSTEM_Initialize();
        TMR1_Init();
        
        delay_ms(1000);
        USBDeviceInit();
        USBDeviceAttach();
        delay_ms(1000);                             //These delays are needed to give the usb time to init
        USBDeviceTasks();
        delay_ms(1000);
        
        int y=0;
        for(y=0; y<64; y++)
            readBuffer[y]='\0';                     //Init usb read buffer as all null
        
        while (1)
        {
        // Add your application code
        DoUSBComms();
        
        }
    }
    
    else if(RCONbits.POR == 1){                     //If system has just powered on then jump to main application
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
//    uint8_t i;
//    int t=0;
    uint8_t numBytesRead;

    numBytesRead = getsUSBUSART(readBuffer, sizeof(readBuffer));
    sscanf(readBuffer, "%s", usbCmd);
    
//    for(t=0; t<numBytesRead; t++){
//        if(readBuffer[t] != '\0')                                 //Scans usb buffer to see if any elements are not null
//        {
//            message_received = true;                              //If not all null then message has been received 
//        }
//    }
//    t = 0;
//    if(message_received == false)                                 //Exit DoUSBComs if no message
//        return;
//    
//    else{
    if(numBytesRead == 4){                                          //Upgrade data sent from QT is 4 8bit values
        volatile uint32_t temp = (uint32_t)readBuffer[0];           //These values are combined into a 32bit int
        upgradeWord= (temp << 24);
        volatile uint32_t temp2 = (uint32_t)readBuffer[1];
        upgradeWord |= temp2<<16;
        volatile uint32_t temp3 = (uint32_t)readBuffer[2];
        upgradeWord |= temp3<<8;
        volatile uint32_t temp4 = (uint32_t)readBuffer[3];
        upgradeWord |= temp4;
    
        uint32_t address = PROGRAM_START_ADDRESS + (FLASH_WORD_SIZE*currentWordWriteCounter);   //Address to write starts at 0x2400 
                                                                                                //Since it writes word by word we increment address by single word size
        FLASH_WriteWord24(address, upgradeWord);

        currentWordWriteCounter++;                                                              //Increment to next word address
        clearUsbBuffers();
        if(address <= 0xABF6){                                                                  //Checks to see if we are at last word write
            if( USBUSARTIsTxTrfReady() == false)
                    CDCTxService();
            putUSBUSART((uint8_t *)"Next\r\n",4);
        }
        else{
            if( USBUSARTIsTxTrfReady() == false)
                    CDCTxService();
            putUSBUSART((uint8_t *)"STOP\r\n",4);                                              //If last word has been written PIC sends stop to QT
        }
    }           
//        if(numBytesRead == 64){
//            for(i=0+upgradeBufferCounter; i<32+upgradeBufferCounter; i+=2){
//            upgradeBuffer[i] = (readBuffer[t]<<32) | ((readBuffer[t+1] & 0xffff)<<16);
//            upgradeBuffer[i+1]= ((readBuffer[t+2] & 0xffff)<<8) | (readBuffer[t+3] & 0xffff);
////            uint16_t temp1 = upgradeBuffer[i]
//            t+=4;
//            }
//            t=0;
//            upgradeBufferCounter += 32;
//            message_received = false;
//            clearUsbBuffers();
//            
//            if(upgradeBufferCounter == 192){
//                if( USBUSARTIsTxTrfReady() == false)
//                    CDCTxService();
//                putUSBUSART((uint8_t *)"STOP\r\n", 4);
//                //write row to flash
//                FLASH_Unlock(FLASH_UNLOCK_KEY);
//                int temp;
//                for(temp=0; temp<192; temp+=2){
//                    uint32_t address = 0x2400 + (0x0002*currentWordWriteCounter);
//                    FLASH_WriteWord24(address, upgradeBuffer[temp]);
//                    currentWordWriteCounter++;
//                }
//                upgradeBufferCounter = 0;
//                delay_ms(1);
//                FLASH_Lock();
////                putUSBUSART((uint8_t *)"NextRow\r\n",7);
//            }
//            
//            else{
//                if( USBUSARTIsTxTrfReady() == false)
//                    CDCTxService();
//                putUSBUSART((uint8_t *)"64\r\n",2);
//                message_received = false;
//                clearUsbBuffers();
//            }     
//        }
    if(strcmp(usbCmd,"FLASH_UNLOCK")==0){
        FLASH_Unlock(FLASH_UNLOCK_KEY);
        clearUsbBuffers();
    }
    
    if(strcmp(usbCmd,"FLASH_LOCK")==0){
        FLASH_Lock();
        clearUsbBuffers();
    }

    if((strcmp(usbCmd,"READ_MEM")==0)){
        uint32_t address = PROGRAM_START_ADDRESS + (FLASH_WORD_SIZE*currentWordReadCounter);        //Address to read starts at 0x2400 
        uint32_t word = FLASH_ReadWord24(address);                                                  //Since it reads word by word we increment address by single word size
            
        uint8_t word8bit[4];
        word8bit[0] = (word & 0xff000000UL) >> 24;                                                  //Flash read word returns a 32bit int
        word8bit[1] = (word & 0x00ff0000UL) >> 16;                                                  //This is split into four 8bits to be sent over usb
        word8bit[2] = (word & 0x0000ff00UL) >>  8;
        word8bit[3] = (word & 0x000000ffUL)      ;
            
        currentWordReadCounter += 1;                                                                //Increments to next word
            
        if(address <= 0xABF6){                                                                      //If not at last word then send
            if( USBUSARTIsTxTrfReady() == false)
                CDCTxService();
            putUSBUSART(word8bit, 4);
        }
            
        else{
            if( USBUSARTIsTxTrfReady() == false)
                CDCTxService();
            putUSBUSART((uint8_t *)"STOP\r\n", 4);                                                 //Sends stop if last word has been read
            if( USBUSARTIsTxTrfReady() == false)
                CDCTxService();
            putUSBUSART((uint8_t *)"STOP\r\n", 4);
        }  
        clearUsbBuffers();
    }
        
    if(strcmp(usbCmd,"ERASE_FLASH")==0){
        uint32_t address = PROGRAM_START_ADDRESS;
        
        FLASH_Unlock(FLASH_UNLOCK_KEY);
        while(address <= 0x0000A7F8){                                                           //Erases flash by page while not at the end
            FLASH_ErasePage(address);                                                           //A page is 0x400 so final address is max memory - 400
            address += FLASH_PAGE_SIZE;                                                         //Address is incremented by page
        }
        FLASH_Lock();
        
        if( USBUSARTIsTxTrfReady() == false)
                CDCTxService();
            putUSBUSART((uint8_t *)"Done_Erase\r\n", 10);                                       //Sends Done erase to QT when finished
        clearUsbBuffers();
    }
    
    if(strcmp(usbCmd,"JUMP_APP")==0){                                           
        currentWordReadCounter = 0;                                             //Resets read and write word counters
        currentWordWriteCounter = 0;
        disableInterrupts();                                                    
        delay_ms(50);
        USBDeviceDetach();                                                      //USB must preform soft detach before jump
        delay_ms(700);                                                          //Needs time for this to happen
        asm("GOTO 0x2600");                                                     //Jump instruction to main app
    }
            
    else if(strcmp(usbCmd,"UPGRADE")==0){
        clearUsbBuffers();
        putUSBUSART((uint8_t *)"NextRow\r\n",7);
    }
    
    else{
        clearUsbBuffers();
    }

    if( USBUSARTIsTxTrfReady() == true)
    {
       
    }
//    }

    CDCTxService();
}

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

