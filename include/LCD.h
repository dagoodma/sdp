/**
 * @file    LCD.h
 * @author  Bryant Mairs
 * @author	Pavlo Manovi
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

#ifndef LCD_H
#define LCD_H


/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/

// LCD modules line length and number of lines
#define LCD_LINE_LENGTH		16
#define LCD_LCD_TOTAL		4

/**
 * 1.64ms delay 
 * This macro defines a short delay of 1.64ms (129 counts of 156,250 Hz clock)
 * in units that are appropriate for the prescalar register of a timer. It can
 * therefore be passed directly into the SET_TIMER2_DELAY() macro.
 */
#define LONG_DELAY	129

/**
 * This macro is similar to LONG_DELAY above, but defines a short 40us delay
 * (4 counts of 156,250 Hz clock).
 */
#define SHORT_DELAY	4

/**
 * This enum contains all the LCD commands that are required for configuring
 * the LCD and updating it. These commands only need a short delay afterwards
 * unless otherwise specified.
 */
enum {
	// Module configuration
	SetDisplayMode = 0x38, // Requires a long delay afterwards
	EnableDisplay = 0x0C,
	SetEntryMode = 0x06,

	// Cursor positioning
	MoveCursorToFirstLine = 0x02, // Requires a long delay afterwards
	MoveCursorToSecondLine = 0xC0
};

/*******************************************************************************
 * Public Functions                                                            *
 ******************************************************************************/
 
/**********************************************************************
 * Function: LCD_init
 * @return None.
 * @remark Initializes the LCD library, which initializes the peripheral
 * 	Timer 2 interrupt to call an update function that displays characters
 *  via a state machine. Interrupt periodicity is defined as SHORT_DELAY.
 *	The low level driver is also initialized by this function.
 * @remark Note that none of the initialization commands are issued within
 *	this function. They are completely handled by an interrupt handler
 * 	that is directly called by Timer 2.	
  **********************************************************************/
void LCD_init(void);

/**********************************************************************
 * Function: LCD_setLine
 * @return None.
 * @remark Sets the characters that will display on the given line of the LCD. As the LCD can
 * 	only hold LCD_LINE_LENGTH characters per line, extra characters beyond these will be ignored.
 * 	If fewer characters than LCD_LINE_LENGTH are passed , it will be assumed that
 * 	the rest of the line should be empty and so it will be cleared appropriately.
 * @remark Reads the input string until a NULL-character is found or LCD_LINE_LENGTH characters
 * 	have been read.
  **********************************************************************/
void LCD_setLine(uint8_t line, const char *newLine);


/**********************************************************************
 * Function: LCD_getLine
 * @return Character array for the given line of the LCD.
 * @remark Returns characters passed in to the LCD_setLine() up to
 * 	LCD_LINE_LENGTH + 1.
 * @remark Character pointer is statically allocated and so should not be
 *  free()d or otherwise modified by the calling code. Use strcpy()d if
 *	desired.
  **********************************************************************/
const char *LCD_getLine(uint8_t line);

#endif // LCD_H
