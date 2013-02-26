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
#include <stdint.h>


/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/

#define DELAY(ms)   do { int i; for (i = 0; i < (ms << 8); i++) { asm ("nop"); } } while(0);

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

// Timer allocation
#define TIMER_BAROMETER         1
#define TIMER_ACCELEROMETER     2
#define TIMER_THERMAL           3
#define TIMER_GPS               4
#define TIMER_BAROMETER2        14 // remove the blocking code!!
#define TIMER_TEST              15


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

/**
 * Function: Board_GetPBClock
 * @return System bus speed in hertz.
 * @author David Goodman
 * @date 2013.01.18  */
uint32_t Board_GetPBClock();

#endif	/* Board_H */

