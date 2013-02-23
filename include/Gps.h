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
 * @note
 *  The longitude and latitude are both in degrees scaled by 1e-7.
 *  The heading is in degrees scaled by 1e-5.
 *  The velocity is in cm/s.
 * 
 * @date January 1, 2013, 1:25 AM   -- Created.
 */
#ifndef Gps_H
#define Gps_H


/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/

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
 * Function: GPS_hasFix
 * @return TRUE if a lock has been obtained.
 * @remark
 **********************************************************************/
BOOL GPS_hasFix();

/**********************************************************************
 * Function: GPS_getLatitude
 * @return The GPS's latitude value in degrees (N/S) scaled 1e-7.
 * @remark
 **********************************************************************/
int32_t GPS_getLatitude();

/**********************************************************************
 * Function: GPS_getLongitude
 * @return The GPS's longitude value in degrees (E/W) scaled 1e-7.
 * @remark
 **********************************************************************/
int32_t GPS_getLongitude();

/**********************************************************************
 * Function: GPS_getAltitude
 * @return The GPS's altitude value in milimeters.
 * @remark
 **********************************************************************/
int32_t GPS_getAltitude();

/**********************************************************************
 * Function: GPS_getNorthVelocity
 * @return Returns the current velocity in the north direction.
 * @remark Centimeters per second in the north direction.
 **********************************************************************/
int32_t GPS_getNorthVelocity();

/**********************************************************************
 * Function: GPS_getEastVelocity
 * @return Returns the current velocity in the east direction.
 * @remark Centimeters per second in the east direction.
 **********************************************************************/
int32_t GPS_getEastVelocity();


/**********************************************************************
 * Function: GPS_getHeading
 * @return Returns the current heading in degrees scaled 1e-5.
 * @remark In degrees scaled by 1e-5.
 **********************************************************************/
int32_t GPS_getHeading();


/**********************************************************************
 * Function: GPS_isConnected
 * @return Returns true if GPS data seen in last 5 seconds.
 * @remark
 **********************************************************************/
int32_t GPS_isConnected();

#endif
