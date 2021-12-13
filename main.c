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

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
//BOOTLOADER
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/usb/usb.h"
#include "timer.h"

void DoUSBComms(void);
void disableInterrupts(void);

//Used for USB
char message_buffer[64];
bool message_received = false;
static uint8_t readBuffer[64];
static uint8_t writeBuffer[64];
/*
                         Main application
 */
int main(void)
{
    // initialize the device
    INTCON2bits.ALTIVT = 1;
    SYSTEM_Initialize();
    TMR1_Init();
    delay_ms(1000);
    USBDeviceInit();
    USBDeviceAttach();
    delay_ms(1000);
    USBDeviceTasks();
    delay_ms(1000);
    while (1)
    {
        // Add your application code
        DoUSBComms();
//        if(USBUSARTIsTxTrfReady())
//        {
//            char bl[] = "UPGRADE";
//            putUSBUSART(bl,8);
//        }
//        int t=0;
//        for(t=0; t<10; t++){
//        int x=0;
//        int y=0;
//        PORTBbits.RB6 = 1;
//        for(x=0; x<1000; x++){
//            for(y=0; y<1000; y++){;}
//        }
//        PORTBbits.RB6 = 0;
//        for(x=0; x<1000; x++){
//            for(y=0; y<1000; y++){;}
//        }
//        }
//        SRbits.IPL0 = 1;
//        SRbits.IPL1 = 1;
//        SRbits.IPL2 = 1;
        disableInterrupts();
        USBDeviceDetach();
        delay_ms(200);
        asm("GOTO 0x2400");
    }

    return 1;
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

    if( USBUSARTIsTxTrfReady() == true)
    {
        uint8_t i;
        uint8_t numBytesRead;

        numBytesRead = getsUSBUSART(readBuffer, sizeof(readBuffer));

        for(i=0; i<numBytesRead; i++)
        {
            message_buffer[i] = readBuffer[i];
                if(message_buffer[i] == '.')
                {
                    message_received = true;
                    putUSBUSART((uint8_t *)"Received full stop.\r\n",19);
                    return;
                }
                            
            switch(readBuffer[i])
            {
                /* echo line feeds and returns without modification. */
                case 0x0A:
                case 0x0D:
                    writeBuffer[i] = readBuffer[i];
                    break;

                /* all other characters get +1 (e.g. 'a' -> 'b') */
                default:
                    writeBuffer[i] = readBuffer[i] + 1;
                    break;
            }
        }

        if(numBytesRead > 0)
        {
            putUSBUSART(writeBuffer,numBytesRead);
        }
    }

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

