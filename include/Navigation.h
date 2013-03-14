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

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/

#define PI                      3.14159265359
#define DEGREE_TO_RADIAN(deg)   ((float)deg*PI/180)
#define RADIAN_TO_DEGREE(rad)   ((float)rad*180/PI)

#define DEGREE_TO_NEDFRAME(deg) (-(deg + 180.0))

// Angle limits 
#define YAW_LIMIT       360.0f // (non-inclusive)
#define PITCH_LIMIT     90.0f  // (inclusive)

/***********************************************************************
 * PUBLIC TYPEDEFS
 ***********************************************************************/

// Geodetic (lat, lon, alt) or NED coordinate for GPS
typedef struct oCoordinate {
    float x, y ,z;
} Coordinate;

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/
/**
 * Function: Navigation_init
 * @return SUCCESS or FAILURE.
 * @remark Initializes the navigation system by intializing the GPS module.
 * @author David Goodman
 * @date 2013.03.10  */
BOOL Navigation_init();
   
/**
 * Function: Navigation_runSM
 * @return None.
 * @remark Wrapper around GPS_runSM(), which executes a cycle of the
 *  navigation state machine.
 * @author David Goodman
 * @date 2013.03.10  */
void Navigation_runSM();

/**
 * Function: Navigation_isReady
 * @return TRUE or FALSE if the Navigation system is ready.
 * @remark Returns TRUE if the GPS sub-system has a fix and a current 
 *  geodetic position.
 * @author David Goodman
 * @date 2013.03.10  */
BOOL Navigation_isReady();

#ifdef IS_COMPAS
/**
 * Function: Navigation_getProjectedCoordinate
 * @param A new geodetic coordinate to save the result into.
 * @param Yaw angle to projected position in degrees.
 * @param Pitch angle to projected position in degrees.
 * @param Height from projected position in degrees
 * @return SUCCESS or FAILURE.
 * @remark Converts the given euler angles to NED, then to ECEF, then adds
 *  them to the current ECEF location, and convert to geodetic (LLA).
 * @author David Goodman
 * @date 2013.03.10  */
BOOL Navigation_getProjectedCoordinate(Coordinate *coord, float yaw, float pitch, float height);
#endif

/**
 * Function: Coordinate_new
 * @param New Coordinate.
 * @param X position (either latitude, ecef_x, or north).
 * @param Y position (either longitude, ecef_y, or east).
 * @param Z position (either altitude, ecef_z, or down).
 * @return The Coordinate object pointer.
 * @remark Constructor for a geodetic, NED, or ECEF coordinate.
 * @author David Goodman
 * @date 2013.03.10  */
//Coordinate Coordinate_new(Coordinate coord, float x, float y, float z);

#endif // Navigation_H

