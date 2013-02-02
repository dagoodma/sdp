/*
 * @file  Xbee.h
 *
 * @author John Ash
 *
 * @brief
 * State machine for Xbee module.
 *
 * @details
 * Module that wraps the bee in a statemachine that
 * reads from the UART.
 *
 * @date February 1, 2013 2:59 AM -- created
 *
 */
#ifndef Xbee_H
#define Xbee_H



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
 * @param N/A
 * @return Failure or Success
 * @remark Initializes the Xbee module. Sets up API mode if the flag is
 * enabled. The function than initializes the sendArray.
 * @author John Ash
 * @date February 1st 2013
 **********************************************************************/
uint8_t Xbee_init();

/**********************************************************************
 * Function: Xbee_programInit()
 * @return Success or Failure based on weather API mode could be set.
 * @remark Puts the Xbee into API mode. Will only be called if
 *  REPROGRAM_API is defined
 * @author John Ash
 * @date February 1st 2013
 **********************************************************************/
uint8_t Xbee_programApi();

/**********************************************************************
 * Function: Xbee_sendString()
 * @param Array of data that is intended to be sent over Xbee
 * @param Length of Data array in bytes
 * @return none
 * @remark Sends data over the Xbee
 * @author John Ash
 * @date February 1st 2013
 **********************************************************************/
void Xbee_sendData(char* data, int Length);


/**********************************************************************
 * Function: void Xbee_runSM();
 * @remark This function should be called every iteration of the
 *  statemachine. We load the recieve array with the data bytes, if we
 *  are actually reading a packet and not just noise.
 * @return none
 * @author John Ash
 * @date February 1st 2013
 **********************************************************************/
void Xbee_runSM();

/**********************************************************************
 * Function: Xbee_isInitialized()
 * @return Whether the Xbee was initialized.
 * @author John Ash
 * @date February 1st 2013
 **********************************************************************/
uint8_t Xbee_isInitialized(void);

/**********************************************************************
 * Function: Xbee_hasNewPacket()
 * @return Whether the recieve array has a new packet in it
 * @author John Ash
 * @date February 1st 2013
 **********************************************************************/
uint8_t Xbee_hasNewPacket(void);

#endif