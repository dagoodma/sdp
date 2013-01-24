/* 
 * File:   Board.h
 * Author: Max Dunne
 * Edited by: David Goodman
 *
 * Edited on January 1, 18, 2013
 * Created on December 19, 2012, 2:08 PM
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
#define FALSE ((int8_t) 0)
#define TRUE ((int8_t) 1)
#define ERROR ((int8_t) -1)
#define SUCCESS ((int8_t) 0)
#define FAILURE ((int8_t) 1)


// Timer allocation
#define TIMER_BAROMETER         1
#define TIMER_ACCELEROMETER     2
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
void Board_init();
uint32_t Board_GetPBClock();

#endif	/* Board_H */

