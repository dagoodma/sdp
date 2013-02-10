/**
 * @file   Serial.h
 * @author Max Dunne
 *
 * @brief
 * Interace for communicating serially over UART.
 *
 * @date January, 18, 2013, 11:52 PM    -- Edited
 * @date November 10, 2011, 8:43 AM     -- Created
 */

#ifndef Serial_H
#define Serial_H


/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/


/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/

/**
 * Function: Serial_init
 * @param None
 * @return SUCCESS or ERROR.
 * @remark Initializes the UART subsystem to 115200 and sets up the circular buffer
 * @author Max Dunne
 * @date 2011.11.10  */
char Serial_init(void);

/**
 * Function: Serial_initSM
 * @param Whether printf will act as blocking code.
 * @return SUCCESS or ERROR.
 * @remark If non-blocking serial use is desired, Serial_runSM must be called
 *      repeatability to ensure all bytes are sent.
 * @author David Goodman
 * @date 2013.02.09  */
char Serial_initSM();

/**
 * Function: Serial_putChar
 * @param ch, the char to be sent
 * @return None
 * @remark adds character to circular buffer and starts the uart transmitting
 *          if not already
 * @author Max Dunne
 * @date 2011.11.10  */
void Serial_putChar(char ch);

/**
 * Function: Serial_getChar
 * @param None
 * @return character or 0
 * @remark retrieves first character from the receive buffer or 0
 * @author Max Dunne
 * @date 2011.11.10  */
char Serial_getChar(void);

/**
 * Function: Serial_isTransmitEmpty
 * @param None
 * @return TRUE or FALSE
 * @remark returns the state of the transmit buffer
 * @author Max Dunne
 * @date 2011.12.15  */
char Serial_isTransmitEmpty(void);

/**
 * Function: Serial_isReceiveEmpty
 * @param None
 * @return TRUE or FALSE
 * @remark returns the state of the receive buffer
 * @author Max Dunne
 * @date 2011.12.15  */
char Serial_isReceiveEmpty(void);

#endif // Serial_H
