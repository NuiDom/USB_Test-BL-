/* 
 * File:   upgradesystems.h
 * Author: root
 *
 * Created on 20 December 2021, 11:40
 */

#ifndef UPGRADESYSTEMS_H
#define	UPGRADESYSTEMS_H

#ifdef	__cplusplus
extern "C" {
#endif

#define PROGRAM_START_ADDRESS   0x00002400
#define FLASH_PAGE_SIZE         0x400
#define FLASH_ROW_SIZE          0x80
    
typedef enum
{
    erase,
    write,
    read,
    alive,
    jump        
}upgradeCmds;

upgradeCmds usb;

void interpretCmds(void);




#ifdef	__cplusplus
}
#endif

#endif	/* UPGRADESYSTEMS_H */

