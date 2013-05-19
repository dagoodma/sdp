/**
 * @file    Navigation.h
 * @author  David Goodman
 *
 * @brief
 * Navigation system interface for COMPAS and ATLAS systems.
 *
 * @details
 * This module utilizes the GPS module to provide functions
 * for the navigation systems of the COMPAS and ATLAS. IS_COMPAS
 * or IS_ATLAS must be defined in order for this module to provide
 * functionality.
 *
 * @date March 10, 2013, 10:03 AM  -- Created
 */

#ifndef Navigation_H
#define Navigation_H

#include <stdint.h>
#include <math.h>
#include "GPS.h"
#include "Error.h"

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/



/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

/**********************************************************************
 * Function: Navigation_init
 * @return TRUE or FALSE whether initialization succeeded.
 * @remark Initializes the navigation state machine.
 **********************************************************************/
bool Navigation_init();


/**********************************************************************
 * Function: Navigation_runSM
 * @return None
 * @remark Steps through the navigation state machine by one cycle.
 **********************************************************************/
void Navigation_runSM();


/**********************************************************************
 * Function: Navigation_gotoLocalCoordinate
 * @param
 * @return None
 * @remark Starts navigating to the desired location until within the given
 *  tolerance range.
 **********************************************************************/
void Navigation_gotoLocalCoordinate(LocalCoordinate *ned_des, float tolerance);


/**********************************************************************
 * Function: Navigation_getLocalDistance
 * @param A pointer to a local coordinate point.
 * @return Distance to the point in meters.
 * @remark Calculates the distance to the given point from the current
 *  position (in the local frame).
 **********************************************************************/
float Navigation_getLocalDistance(LocalCoordinate *nedPoint);


/**********************************************************************
 * Function: Navigation_getLocalPosition
 * @param A pointer to a local coordinate point.
 * @return None
 * @remark Saves the current local (NED) position into the given variable.
 **********************************************************************/
void Navigation_getLocalPosition(LocalCoordinate *nedPosition);


/**********************************************************************
 * Function: Navigation_setOrigin
 * @param A pointer to geocentric coordinate location.
 * @return None
 * @remark Sets the geodetic and ECEF origin point (generally the location
 *  of the command center), by calculating the geodetic from the given
 *  ECEF coordinate.
 **********************************************************************/
void Navigation_setOrigin(GeocentricCoordinate *ecefRef);


/**********************************************************************
 * Function: Navigation_setGeocentricError
 * @param Geocentric error to add to measured geocentric position.
 * @return None
 * @remark Sets the geocentric error for error corrections.
 **********************************************************************/
void Navigation_setGeocentricError(GeocentricCoordinate *error);


/**********************************************************************
 * Function: Navigation_cancel
 * @return None
 * @remark Cancels the current mission ,if navigating to a location,
 *  and stops the motors and centers the rudder.
 **********************************************************************/
void Navigation_cancel();


/**********************************************************************
 * Function: Navigation_enableErrorCorrection
 * @return None
 * @remark Enables error correction for retreived coordinates.
 **********************************************************************/
void Navigation_enableErrorCorrection();

/**********************************************************************
 * Function: Navigation_disableErrorCorrection
 * @return None
 * @remark Disables error correction for retreived coordinates.
 **********************************************************************/
void Navigation_disableErrorCorrection();


/**********************************************************************
 * Function: Navigation_isReady
 * @return True if we are ready to navigate with GPS and have an origin.
 * @remark
 **********************************************************************/
bool Navigation_isReady();


/**********************************************************************
 * Function: Navigation_hasError
 * @return TRUE or FALSE if an error occuirred while navigating.
 * @remark Errors occur if the GPS has lost a fix or become disconnected.
 *  Error codes are defined in Error.h, and can be obtained with the
 *  Navigation_getError() function.
 **********************************************************************/
bool Navigation_hasError();


/**********************************************************************
 * Function: Navigation_getError
 * @return Error code corresponding to the last error experienced
 *  while navigating.
 * @remark Error codes are defined in Error.h. Note that this function
 *  clears the error. Also, using Navigation_gotoLocalCoordinate will
 *  clear any error codes.
 **********************************************************************/
error_t Navigation_getError();


/**********************************************************************
 * Function: Navigation_isDone
 * @return TRUE or FALSE whether we successfully navigated to a
 *  desired location.
 * @remark 
 **********************************************************************/
bool Navigation_isDone();


/**********************************************************************
 * Function: Navigation_isNavigating
 * @return TRUE or FALSE whether we are navigating to a location.
 * @remark 
 **********************************************************************/
bool Navigation_isNavigating();

/**********************************************************************
 * Function: Navigation_isUsingErrorCorrection
 * @return TRUE or FALSE whether error correction is enabled.
 * @remark 
 **********************************************************************/
bool Navigation_isUsingErrorCorrection();


#endif // Navigation_H

