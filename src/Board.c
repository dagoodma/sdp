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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "Board.h"
#include "Serial.h"
#include "Uart.h"
#include "Timer.h"
#include "LCD.h"

#define SYSTEM_CLOCK    80000000L
#define PB_CLOCK        SYSTEM_CLOCK/2


#define STARTUP_DELAY   2000 // (ms) to spend configuring (for LCD to power up)

struct {
    unsigned int useSerial :1;
    unsigned int useLCD :1;
    unsigned int useTimer :1;
} option;

void Board_init()
{
    INTEnableSystemMultiVectoredInt();
}

void Board_configure(uint8_t opts) {
    DELAY(STARTUP_DELAY );
    if (opts & USE_TIMER) {
        Timer_init();
    }
    if (opts & USE_SERIAL) {
        option.useSerial = TRUE;
        Serial_init();
    }
    if (opts & USE_LCD) {
        option.useLCD = TRUE;
        LCD_init();
    }
    DELAY(STARTUP_DELAY );
}

unsigned int Board_GetPBClock()
{
    return PB_CLOCK;
}

void delayMillisecond(int ms) {
    Timer_new(TIMER_DELAY, ms);
    while (!Timer_isExpired(TIMER_DELAY))
        asm("nop");
}


void dbprint(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char msg[255];
    vsprintf(msg, fmt, args);

    if (option.useSerial)
        printf(msg);
    if (option.useLCD)
        LCD_writeString(msg);
    #ifdef USE_LOGGER
    Logger_writeString(msg);
    #endif
};


//#define BOARD_TEST
#ifdef BOARD_TEST


int main(void)
{
    Board_init();
    Serial_init();
    printf("If you can see this it worked");
}


#endif
