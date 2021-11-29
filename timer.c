#include "timer.h"  
#include "mcc_generated_files/system.h"

void TMR1_Init()
{
    TMR1 = 0;
    PR1 = 0xFFFF;
    T1CON = 0x8030;
}

void delay_ms(int delay)
{
    float tmp = delay/1000.0;
    while(TMR1<SEC*tmp){}
    TMR1 = 0;
}