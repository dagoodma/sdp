/**
 * @file    Gps.h
 * @author  David Goodman
 *
 * @brief
 * Sensor interface for the GPS.
 *
 * @details 
 * Module that wraps the GPS in a state machine, which ocassionally
 * pulls location and velocity data from the GPS over UART. Also, this
 * module can be configured to communicate with a command station to
 * reduce error and improve accuracy.
 * 
 * @date January 1, 2013, 1:25 AM   -- Created.
 */
#ifndef Gps_H
#define Gps_H


/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/


/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/


/**********************************************************************
 * Function: GPS_init
 * @param An options bitfield.
 * @return none
 * @remark Initializes the GPS module.
 **********************************************************************/
void GPS_init(uint8_t options);

/**********************************************************************
 * Function: GPS_isInitialized
 * @return Whether the GPS was initialized.
 * @remark none
 **********************************************************************/
bool GPS_isInitialized();

#endif
