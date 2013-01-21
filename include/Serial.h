/*
 *  File:   Serial.h
 *  Author: mdunne
 *
 *  Edited on January, 18, 2013, 11:52 PM
 *  Created on November 10, 2011, 8:43 AM
 */

#ifndef Serial_H
#define Serial_H


/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/


/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/


/****************************************************************************
 Function
     

 Parameters
     none

 Returns
     None.

 Description
    
 Notes
     None.

 Author
 Max Dunne, 2011.11.10 0905
 ****************************************************************************/
/**
 * Function: Serial_Init
 * @param None
 * @return SUCCESS or ERROR.
 * @remark Initializes the UART subsystem to 115200 and sets up the circular buffer
 * @author Max Dunne
 * @date 2011.11.10  */
char Serial_Init(void);

/**
 * Function: Serial_PutChar
 * @param ch, the char to be sent
 * @return None
 * @remark adds character to circular buffer and starts the uart transmitting
 *          if not already
 * @author Max Dunne
 * @date 2011.11.10  */
void Serial_PutChar(char ch);

/**
 * Function: Serial_GetChar
 * @param None
 * @return character or 0
 * @remark retrieves first character from the receive buffer or 0
 * @author Max Dunne
 * @date 2011.11.10  */
char Serial_GetChar(void);

/**
 * Function: Serial_IsTransmitEmpty
 * @param None
 * @return TRUE or FALSE
 * @remark returns the state of the transmit buffer
 * @author Max Dunne
 * @date 2011.12.15  */
char Serial_IsTransmitEmpty(void);

/**
 * Function: Serial_IsReceiveEmpty
 * @param None
 * @return TRUE or FALSE
 * @remark returns the state of the receive buffer
 * @author Max Dunne
 * @date 2011.12.15  */
char Serial_IsReceiveEmpty(void);

#endif // Serial_H
