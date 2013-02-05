/**
 * @file    UART.h
 * @author  John Ash
 * @author  David Goodman
 * @author  Max Dunne
 *
 * @brief
 * Interface for communicating over a UART.
 *
 * @details
 * This module is used for communicating with another device over UART.
 *
 * @date January 23, 2013   -- Created
 */


#define UART1_ID 1
#define UART2_ID 2
#define UART_SERIAL_ID UART1_ID

/**
 * Function: UART_init()
 * @param id: identifies the UART module we want to initialize.
 *        baudRate: The baudRate we want to initialize the function to.
 * @return None
 * @remark Initializes the UART to the specific ID and BaudRate
 * @author Max Dunne
 * @author John Ash
 * @date February 1st, 2013 */

void UART_init(uint8_t id, uint32_t baudRate);

/**
 * Function: UART_putChar
 * @param identifies the UART module
 * @param the char to be sent
 * @return None
 * @remark adds character to circular buffer and starts the uart transmitting
 *          if not already
 * @author Max Dunne
 * @author John Ash
 * @date February 1st, 2013 */
void UART_putChar(uint8_t id, char ch);


/**
 * Function: UART_putChar
 * @param identifies the UART module
 * @param an array of character to be sent
 * @param The length of the array
 * @return None
 * @remark adds character to circular buffer and starts the uart transmitting
 *          if not already
 * @author John Ash
 * @date February 1st, 2013 */
void UART_putString(uint8_t id, char* Data, int Length);


/**
 * Function: UART_getChar
 * @param identifies the UART module
 * @return 0xFF00 if the data is not valid, and 0x00FF & data if the
 *      data is valid. It is the job of the function caller to check
 *      for valid data.
 * @remark retrieves first character from the receive buffer or 0. Currently
 *      uses blocking code between each character.
 * @author Max Dunne
 * @author John Ash
 * @date February 1st, 2013  */
uint16_t UART_getChar(uint8_t id);

/**
 * Function: UART_isTransmitEmpty
 * @param identifies the UART module
 * @return TRUE or FALSE
 * @remark returns the state of the transmit buffer
 * @author Max Dunne
 * @author John Ash
 * @date February 1st, 2013  */
char UART_isTransmitEmpty(uint8_t id);

/**
 * Function: UART_isReceiveEmpty
 * @param identifies the UART module
 * @return TRUE or FALSE
 * @remark returns the state of the receive buffer
 * @author Max Dunne
 * @author John Ash
 * @date February 1st, 2013  */
char UART_isReceiveEmpty(uint8_t id);
