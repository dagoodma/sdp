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
BOOL GPS_init(uint8_t options);

/**********************************************************************
 * Function: GPS_isInitialized()
 * @return Whether the GPS was initialized.
 * @remark none
 **********************************************************************/
BOOL GPS_isInitialized();

/**********************************************************************
 * Function: GPS_runSM()
 * @return None
 * @remark Executes the GPS's currently running state.
 **********************************************************************/
void GPS_runSM();

/**********************************************************************
 * Function: GPS_hasLock
 * @return TRUE if a lock has been obtained.
 * @remark
 **********************************************************************/
BOOL GPS_hasLock();

/**********************************************************************
 * Function: GPS_getLatitude
 * @return The GPS's latitude value (N/S) scaled 1e7.
 * @remark
 **********************************************************************/
int32_t GPS_getLatitude();

/**********************************************************************
 * Function: GPS_getLongitude
 * @return The GPS's longitude value (E/W) scaled 1e7.
 * @remark
 **********************************************************************/
int32_t GPS_getLongitude();

#endif
