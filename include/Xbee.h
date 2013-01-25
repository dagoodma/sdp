/**
 * @file    Xbee.h
 * @author  John Ash
 * @author  David Goodman
 *
 * @brief
 * Interface for communicating wirelessly with XBee.
 *
 * @details
 * This module wraps a XBee device in a state machine and uses
 * UART to communicate with other XBee devices wirelessly.
 *
 *
 * @date January 23, 2013   -- Created
 */
#ifndef Xbee_H
#define Xbee_H

#include "Error.h"
#include "Util.h"

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/
#define GROUND_STATION_XBEE 74
#define BOAT_XBEE 33

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/


/**********************************************************************
 * Function: Xbee_init()
 * @param An options bitfield.
 * @return none
 * @remark Initializes the GPS module.
 **********************************************************************/
void Xbee_init(uint8_t options);

/**********************************************************************
 * Function: Xbee_isInitialized()
 * @return Whether the GPS was initialized.
 * @remark none
 **********************************************************************/
bool Xbee_isInitialized();

/**********************************************************************
 * Function: Xbee_programInit()
 * @param Which Xbee is being programed, ground station or boat
 * @remark Puts the Xbee into API mode.
 **********************************************************************/
void Xbee_programInit(uint8 whichXbee);

/**********************************************************************
 * Function: Xbee_isApi()
 * @return Whether the Xbee was succesfully put into API mode
 * @remark none
 **********************************************************************/
bool Xbee_isApi();

/**********************************************************************
 * Function: Xbee_programInit()
 * @param Which Xbee is being programed, ground station or boat
 * @remark Initializes an array with data required for sending a
 *  message in API mode.
 **********************************************************************/
void Xbee_initSend(uint8 whichXbee);

/**********************************************************************
 * Function: Xbee_sendString()
 * @param data that needs to be transmitted, split up into proper bytes
 * @remark Adds data to the sendArray, than sends that data over the 
 *  UART.
 **********************************************************************/
void Xbee_sendString(string data);


/**********************************************************************
 * Function: Xbee_recieveData()
 * @remark Beings taking in data, once a packet has been fully read in,
 * we will analyze that packet for it's useful information, namely
 * the GPS cordinate we should be driving towards.
 **********************************************************************/
void Xbee_recieveData();

#endif // Xbee_H
