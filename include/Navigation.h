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

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/



/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/
BOOL Navigation_init();

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
 * Function: Navigation_setOrigin
 * @return None
 * @remark Sets the longitudal error for error corrections.
 **********************************************************************/
void Navigation_setOrigin(GeocentricCoordinate *ecefRef,
    GeodeticCoordinate *llaRef);

/**********************************************************************
 * Function: Navigation_setGeocentricError
 * @param Geocentric error to add to measured geocentric position.
 * @return None
 * @remark Sets the geocentric error for error corrections.
 **********************************************************************/
void Navigation_setGeocentricError(GeocentricCoordinate *error);

void Navigation_cancel();


/**********************************************************************
 * Function: Navigation_enableErrorCorrection
 * @return None
 * @remark Enables error correction for retreived coordinates.
 **********************************************************************/
void Navigation_enablePositionErrorCorrection();

/**********************************************************************
 * Function: Navigation_disableErrorCorrection
 * @return None
 * @remark Disables error correction for retreived coordinates.
 **********************************************************************/
void Navigation_disablePositionErrorCorrection();


/**********************************************************************
 * Function: Navigation_isReady
 * @return True if we are ready to navigate with GPS and have an origin.
 * @remark
 **********************************************************************/
BOOL Navigation_isReady();

BOOL Navigation_hasError();

BOOL Navigation_clearError();

BOOL Navigation_isNavigating();

BOOL Navigation_isDone();


#endif // Navigation_H

