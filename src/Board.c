/*
 * File:   Board.c
 * Author: David Goodman
 *
 * Created on January 18, 2013, 11:45 PM
 */
/**********************************************************************
 Module
 Board.c

 Revision
   1.0.0

 Description
   This is the WARPbezel LED library.

 Notes
 This the UNO32 device library.

 History
 When           Who         What/Why
 -------------- ---         --------
 01-19-13 12:22 dagoodma    Created file.
***********************************************************************/
#define Board_H_PRIVATE_INCLUDE
#include "Board.h"
//#include "Serial.h"
#include "Uart.h"
//#include <plib.h>

#define SYSTEM_CLOCK    80000000L
#define PB_CLOCK        SYSTEM_CLOCK/2

#define LED1_

void Board_init()
{
    //unsigned int pbclk=PB_CLOCK;
    //SYSTEMConfig(SYSTEM_CLOCK,SYS_CFG_ALL);
    //SYSTEMConfigPB(PB_CLOCK);
    //OSCSetPBDIV(2);
    //Serial_init();
    INTEnableSystemMultiVectoredInt();
}

unsigned int Board_GetPBClock()
{
    return PB_CLOCK;
}



//#define BOARD_TEST
#ifdef BOARD_TEST


int main(void)
{
    Board_init();
        
    printf("If you can see this it worked");
}


#endif
