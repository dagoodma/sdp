/**
 * @file    Board.h
 * @author  Max Dunne
 * @author  David Goodman
 *
 * @brief
 * Interface for PIC32 board.
 *
 * @details
 * Contains functions and definitions for a PIC32 board.
 *
 * 
 * @date January 1, 2013            -- Edited
 * @date December 19, 2012, 2:08 PM -- Created
 */

#ifndef Board_H
#define	Board_H

#include <xc.h>
#include <plib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "Serial.h"
#include "Error.h"
#include "Timer.h"
#include "LCD.h"
#include "Lcd.h"

#ifdef USE_SD_LOGGER
#include "Logger.h"
#endif


/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/

#define DELAY(ms)   do { int i; for (i = 0; i < (ms << 8); i++) { asm ("nop"); } } while(0);

#define MS_TO_SEC(ms)  ((float)ms/1000)

// Debugging statements over terminal, SD logger, or disabled (respectively)
#ifdef DEBUG
#ifdef USE_SD_LOGGER
#define DBPRINT(...)   do { char debug[255]; sprintf(debug,__VA_ARGS__); } while(0)
#else
#define DBPRINT(...)   printf(__VA_ARGS__)
#endif
#else   
#define DBPRINT(...)   ((int)0)
#endif

/*****************************************************************************/
// Boolean defines for TRUE, FALSE, SUCCESS and ERROR
#ifndef TRUE
#define FALSE ((int8_t) 0)
#define TRUE ((int8_t) 1)
#endif

#ifndef ERROR
#define ERROR ((int8_t) -1)
#endif

#ifndef SUCCESS
#define SUCCESS ((int8_t) 0)
#define FAILURE ((int8_t) 1)
#endif

#ifndef OUTPUT
#define OUTPUT  (0)
#define INPUT   (1)
#endif

#ifndef ON
#define ON      (1)
#define OFF     (0)
#endif

// Timer allocation
#define TIMER_BAROMETER         1
#define TIMER_ACCELEROMETER     2
#define TIMER_THERMAL           3
#define TIMER_GPS               4
#define TIMER_ENCODER           5
#define TIMER_BUTTONS           6
#define TIMER_DRIVE             7
#define TIMER_OVERRIDE          8
#define TIMER_TILTCOMPASS       9
#define TIMER_NAVIGATION        10
#define TIMER_LOGGER            11
#define TIMER_BAROMETER2        12 // remove the blocking code!!
#define TIMER_DELAY             13
#define TIMER_INTERFACE         14
#define TIMER_LIGHT_HOLD        15
#define TIMER_LCD_HOLD          16
#define TIMER_I2C_TIMEOUT       17
#define TIMER_RESET             18 

// Master state machine timers
#define TIMER_MAIN              23
#define TIMER_MAIN2             24
#define TIMER_MAIN3             25
#define TIMER_BACKGROUND        26
#define TIMER_BACKGROUND2       27
#define TIMER_BACKGROUND3       28

// test harness timers
#define TIMER_TEST              29
#define TIMER_TEST2             30
#define TIMER_TEST3             31


#ifdef Board_H_PRIVATE_INCLUDE

/*
 LED5 => #58 => RA2
 */
#define BOARD_LED1_TRIS
#define BOARD_LED2_TRIS
#define BOARD_LED3_TRIS
#define BOARD_LED4_TRIS
#define BOARD_LED5_TRIS

#define BOARD_LED1_LAT
#define BOARD_LED2_LAT
#define BOARD_LED3_LAT
#define BOARD_LED4_LAT
#define BOARD_LED5_LAT

#endif


// Options
#define USE_SERIAL      0x1
#define USE_LCD         0x2
#define USE_TIMER       0x4

/*******************************************************************************
 * Public Functions                                                            *
 ******************************************************************************/
/**
 * Function: Board_init
 * @return None.
 * @remark Initializes the Serial interface and enables interrupts.
 * @author David Goodman
 * @date 2013.01.18  */
void Board_init();

void Board_configure(uint8_t opts);

/**
 * Function: Board_GetPBClock
 * @return System bus speed in hertz.
 * @author David Goodman
 * @date 2013.01.18  */
uint32_t Board_GetPBClock();

/**
 * Function: delayMillisecond
 * @param Number of millisecond to delay.
 * @return None
 * @remark Blocking delay using timer module.
 * @author David Goodman
 * @date 2013.04.11  */
void delayMillisecond(int ms);

void dbprint(char *fmt, ...);



#endif	/* Board_H */

