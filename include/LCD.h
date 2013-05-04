/**
 * @file    LCD.h
 * @author  Bryant Mairs
 * @author	Pavlo Manovi
 * @author  David Goodman
 *
 * @brief
 * Interface to an LCD display.
 *
 * @details
 * Contains functions for writing strings to an LCD display. This
 * deals with a 4x20, TM204JAA7 LCD module paticularly, but is
 * made as generic as possible to work with an m by n display.
 *
 * 
 * @date April 17, 2013         -- Created module.
 */

#ifndef LCD_H
#define LCD_H
#include <stdint.h>


/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/

// LCD modules line length and number of lines
#define LCD_LINE_LENGTH		20
#define LCD_LINE_TOTAL		4
#define LCD_CHARACTER_TOTAL     (LCD_LINE_TOTAL*LCD_LINE_LENGTH)

#define LCD_START_LINE1  0x00     /**< DDRAM address of first char of line 1 */
#define LCD_START_LINE2  0x40     /**< DDRAM address of first char of line 2 */
#define LCD_START_LINE3  0x14     /**< DDRAM address of first char of line 3 */
#define LCD_START_LINE4  0x54     /**< DDRAM address of first char of line 4 */

// LCD pins
#define LCD_CGRAM             6      /* DB6: set CG RAM address             */
#define LCD_DDRAM             7      /* DB7: set DD RAM address             */
#define LCD_BUSY              7      /* DB7: LCD is busy                    */

/**
 * This enum contains all the LCD commands that are required for configuring
 * the LCD and updating it. These commands only need a short delay afterwards
 * unless otherwise specified.
 */
enum {
	// Module configuration
	SetDisplayMode = 0x38, // Requires a long delay afterwards
	EnableDisplay = 0x0C, // Display on, cursor and blinking off
	SetEntryMode = 0x06, // Set moving direction of cursor and display (Inc)
        ClearDisplay = 0x01, // writes 0x20 to DDRAM and sets address to 0x00

	// Cursor positioning
        ReturnHome = 0x02, // Requires a long delay afterwards
        MoveCursorToLine1 = 0x00,
        MoveCursorToLine2 = 0x40,
        MoveCursorToLine3 = 0x14,
        MoveCursorToLine4 = 0x54,
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
 * Function: LCD_writeString
 * @param String to show on LCD display.
 * @return None.
 * @remark Writes the given string to the display, and creates a newline for
 *  every newline character '\n'.
  **********************************************************************/
void LCD_writeString(const char *s);

/**********************************************************************
 * Function: LCD_clearDisplay
 * @return None.
 * @remark Clears all text on the LCD display.
  **********************************************************************/
void LCD_clearDisplay();

/**********************************************************************
 * Function: LCD_setPosition
 * @param Line to set the cursor to.
 * @param Character position to set the cursor to.
 * @return None.
 * @remark Sets the cursor to the given position for writing.
  **********************************************************************/
void LCD_setPosition(uint8_t line, uint8_t col);


#endif // LCD_H
