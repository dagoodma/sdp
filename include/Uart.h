/**
 * @file    UART.h
 * @author  John Ash
 * @author  David Goodman
 *
 * @brief
 * Interface for communicating over a UART.
 *
 * @details
 * This module is used for communicating with another device over UART.
 *
 * @date January 23, 2013   -- Created
 */

/**********************************************************************
 * Function: UART_init()
 * @param id: identifies the UART module we want to initialize
 *        baudRate: The baudRate we want to initialize the function too.
 * @return a UINT32 value with the actual baudrate initialized. Check against
 *  desired to see if the system can handle the desired baud rate.
 * @remark Initializes the UART to the specific ID and BaudRate given as
 *  functions to the system. Will return the actual baudrate initialzied.
 **********************************************************************/

uint32_t UART_init(UART_MODULE id, uint32_t baudRate);

/**
 * Function: Serial_putChar
 * @param ch, the char to be sent
 * @return None
 * @remark adds character to circular buffer and starts the uart transmitting
 *          if not already
 * @author Max Dunne
 * @date 2011.11.10  */
void UART_putChar(uint8_t id, char ch);

/**
 * Function: Serial_getChar
 * @param None
 * @return character or 0
 * @remark retrieves first character from the receive buffer or 0
 * @author Max Dunne
 * @date 2011.11.10  */
uint16_t UART_getChar(uint8_t id);

/**
 * Function: Serial_isTransmitEmpty
 * @param None
 * @return TRUE or FALSE
 * @remark returns the state of the transmit buffer
 * @author Max Dunne
 * @date 2011.12.15  */
char UART_isTransmitEmpty(uint8_t id);

/**
 * Function: Serial_isReceiveEmpty
 * @param None
 * @return TRUE or FALSE
 * @remark returns the state of the receive buffer
 * @author Max Dunne
 * @date 2011.12.15  */
char UART_isReceiveEmpty(uint8_t id);
